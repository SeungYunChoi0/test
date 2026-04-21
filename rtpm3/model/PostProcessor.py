'''
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
'''

from datetime import datetime
from PyQt5.QtCore import *
import numpy as np
import cv2
import json
import os, sys

class PostProcessor(QThread):
	def __init__(self, frameWidth, frameHeight):
		self.colorList = [(255, 0, 0), (0, 255, 0), (0, 0, 255)
						, (255, 255, 0), (255, 0, 255)]
		self.resultName = ['OD/TSR', 'LD', 'HBA']
		self.__frameWidth = frameWidth
		self.__frameHeight = frameHeight

		# 21개 손 keypoint 스켈레톤 연결 정의 (MediaPipe Hand landmark 기준)
		# 0=손목, 1-4=엄지, 5-8=검지, 9-12=중지, 13-16=약지, 17-20=소지
		self.HAND_SKELETON = [
			# 손바닥
			(0, 1), (0, 5), (0, 9), (0, 13), (0, 17),
			(5, 9), (9, 13), (13, 17),
			# 엄지
			(1, 2), (2, 3), (3, 4),
			# 검지
			(5, 6), (6, 7), (7, 8),
			# 중지
			(9, 10), (10, 11), (11, 12),
			# 약지
			(13, 14), (14, 15), (15, 16),
			# 소지
			(17, 18), (18, 19), (19, 20),
		]

	def drawBoundingBox(self, frame, odResult, num, drawRatioList=[1.0, 1.0]): # for NN
		resultNum = len(odResult)
		if resultNum > 0:
			for lData in odResult:
				try:
					startPoint = (int(lData['bbox'][0]*drawRatioList[0]), int(lData['bbox'][1]*drawRatioList[1]))
					endPoint = (int((lData['bbox'][0]+lData['bbox'][2])*drawRatioList[0]), int((lData['bbox'][1]+lData['bbox'][3])*drawRatioList[1]))
					cv2.rectangle(frame, startPoint, endPoint, self.colorList[num], 2)
					cv2.putText(frame, '[{}][{:.1f}%]'.format(lData['category_id'], lData['score']), (startPoint[0]-8, startPoint[1]-8), cv2.FONT_HERSHEY_SIMPLEX, 0.6, self.colorList[num], 2)
				except Exception as e:
					_, _ , tb = sys.exc_info()
					print(__name__, tb.tb_lineno, e)
		return frame

	def drawBoundingBoxforDistance(self, frame, odResult, drawRatioList=[1.0, 1.0]): # for Falcon
		resultNum = len(odResult)
		if resultNum > 0:
			for lData in odResult:
				try:
					cId = lData['category_id']
					startPoint = (int(lData['bbox'][0]*drawRatioList[0]), int(lData['bbox'][1]*drawRatioList[1]))
					endPoint = (int((lData['bbox'][0]+lData['bbox'][2])*drawRatioList[0]), int((lData['bbox'][1]+lData['bbox'][3])*drawRatioList[1]))
					cv2.rectangle(frame, startPoint, endPoint, self.colorList[cId], 2)
					if cId != 4:
						cv2.putText(frame, '[{}][{:.1f}%] {:.1f}m'.format(lData['desc'], lData['score'], lData['distance']), (startPoint[0]-8, startPoint[1]-8), cv2.FONT_HERSHEY_SIMPLEX, 0.6, self.colorList[lData['category_id']], 2)
						lData['desc'] = ''
					else:
						cv2.putText(frame, '[{}][{:.1f}%]'.format(lData['desc'], lData['score']), (startPoint[0]-8, startPoint[1]-8), cv2.FONT_HERSHEY_SIMPLEX, 0.6, self.colorList[lData['category_id']], 2)
						lData['distance'] = 0.0
					lData['score'] = lData['score']/100.0
				except Exception as e:
					_, _ , tb = sys.exc_info()
					print(__name__, tb.tb_lineno, e)
		return frame

	def drawLane(self, frame, laneResult, drawRatioList=[1.0, 1.0]):
		resultNum = len(laneResult)
		if resultNum > 0:
			for pointList in laneResult:
				pointList = np.multiply(pointList, drawRatioList)
				pointList = pointList.astype('int32')
				cv2.polylines(frame, [pointList], False, (224, 32, 100), 3)
		return frame

	def printHBA(self, frame, hbaResult):
		if len(hbaResult) > 0:
			# hbaResult['state'] - high / low
			cv2.putText(frame, 'HBA : ' + hbaResult['state'], (50, frame.shape[0] - 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
		return frame

	def printClass(self, frame, clResult, num, posX, posY):
		cv2.putText(frame, 'Cluster_{} : {}'.format(num, clResult), (posX, posY), cv2.FONT_HERSHEY_SIMPLEX, 1, self.colorList[num], 2)
		return frame

	def drawHandPose(self, frame, handsResult, drawRatioList=[1.0, 1.0]):
		"""
		Hand Pose 결과를 프레임에 시각화
		
		Parameters
		----------
		frame        : np.ndarray (BGR 이미지)
		handsResult  : list – RtpmSendHandResultAsJson() 으로 전송된 JSON의
		               'hands' 리스트.
		               각 원소: {'id':int, 'score':float, 'bbox':[x,y,w,h],
		                         'keypoints':[{'id':int,'x':int,'y':int,'conf':float}...]}
		drawRatioList: [x_ratio, y_ratio] – 프레임 디스플레이 스케일 보정

		Returns
		-------
		frame : 시각화가 완료된 np.ndarray
		"""
		if not handsResult:
			return frame

		for hand in handsResult:
			try:
				bbox  = hand.get('bbox', [])
				score = hand.get('score', 0.0)
				kpts  = hand.get('keypoints', [])

				# ── BBox 그리기 ─────────────────────────────
				if len(bbox) == 4:
					x1 = int(bbox[0] * drawRatioList[0])
					y1 = int(bbox[1] * drawRatioList[1])
					x2 = int((bbox[0] + bbox[2]) * drawRatioList[0])
					y2 = int((bbox[1] + bbox[3]) * drawRatioList[1])
					cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
					cv2.putText(frame,
						'hand {:.2f}'.format(score),
						(x1, max(y1 - 6, 10)),
						cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 0), 2)

				# ── Keypoint 좌표 수집 ───────────────────────
				KPT_CONF_THRESH = 0.3
				points = {}  # id → (px, py)
				for kp in kpts:
					kid  = kp.get('id', -1)
					conf = kp.get('conf', 0.0)
					if conf < KPT_CONF_THRESH:
						continue
					px = int(kp.get('x', 0) * drawRatioList[0])
					py = int(kp.get('y', 0) * drawRatioList[1])
					points[kid] = (px, py)

				# ── 스켈레톤 연결선 그리기 ───────────────────
				for (a, b) in self.HAND_SKELETON:
					if a in points and b in points:
						cv2.line(frame, points[a], points[b], (255, 165, 0), 2)

				# ── Keypoint 원 그리기 ───────────────────────
				for kid, (px, py) in points.items():
					# 손목(0)은 크게, 나머지는 작게
					radius = 6 if kid == 0 else 4
					cv2.circle(frame, (px, py), radius, (0, 220, 255), -1)
					cv2.circle(frame, (px, py), radius, (0, 0, 0), 1)  # 외곽선

			except Exception as e:
				_, _, tb = sys.exc_info()
				print(__name__, tb.tb_lineno, e)

		return frame

	def saveResultData(self, fileName, fileDir, imageShape, resultDict):
		data = dict()
		try:
			fileNameList = os.path.splitext(fileName)
			path = os.path.splitdrive(fileDir)
			data['images'] = {'file_path' : fileDir + '/' + fileName}
			data['images']['file_name'] = fileNameList[0]
			data['images']['sub_dir'] = str(path[1][1:])
			data['images']['height'] = imageShape[0]
			data['images']['width'] = imageShape[1]

			height_factor = imageShape[0] / self.__frameHeight
			width_factor = imageShape[1] / self.__frameWidth

			scaledOdList = []
			if 'cluster1' in resultDict and "od" in resultDict['cluster1']:
				for object in resultDict['cluster1']['od']:
					newDict = dict()
					newDict['id'] = object['id']
					newDict['category_id'] = object['category_id']
					newDict['score'] = object['score']
					newDict['desc'] = object['desc']
					newDict['distance'] = object['distance']
					newDict['bbox'] = [round(object['bbox'][0] * width_factor)
										, round(object['bbox'][1] * height_factor)
										, round(object['bbox'][2] * width_factor)
										, round(object['bbox'][3] * height_factor)]
					scaledOdList.append(newDict)

			if 'od' in resultDict:
				for object in resultDict['od']:
					newDict = dict()
					newDict['id'] = object['id']
					newDict['category_id'] = object['category_id']
					newDict['score'] = object['score']
					newDict['desc'] = object['desc']
					newDict['distance'] = object['distance']
					newDict['bbox'] = [round(object['bbox'][0] * width_factor)
										, round(object['bbox'][1] * height_factor)
										, round(object['bbox'][2] * width_factor)
										, round(object['bbox'][3] * height_factor)]
					scaledOdList.append(newDict)

			# data['annotations'] =  resultDict['od']
			data['annotations'] = scaledOdList
			if 'hba' in resultDict:
				data['hba'] = resultDict['hba']
			
			with open('{}_result/data_result/{}.json'.format(fileDir, fileNameList[0]), 'w') as fp:
				json.dump(data, fp, sort_keys=False, indent=4, separators=(',', ':'))

		except Exception as e:
			_, _ , tb = sys.exc_info()
			print(__name__, tb.tb_lineno, e)

	def saveResultList(self, resultNumList, resultList):
		now = datetime.now()
		try:
			f = open(now.strftime("%Y-%m-%d_%H%M") + '.txt', 'w')
			for i in range(len(resultNumList)):
				f.write('[{}] total : {}'.format(self.resultName[i], resultNumList[i]) + '\n')
				for ri in range(len(resultList[i])):
					f.write('({}) : '.format(ri) + str(resultList[i][ri]) + '\n')
			f.close()
		except Exception as e:
			_, _ , tb = sys.exc_info()
			print(__name__, tb.tb_lineno, e)

if __name__ == "__main__":
	img = cv2.imread('test_files/test.jpg')
	dstimg = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
	print(img.shape)
	img.save("test_files/filename.jpeg")
	cv2.imshow('test', dstimg)
	cv2.waitKey()
	cv2.destroyAllWindows()