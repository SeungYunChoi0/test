Write-Host "========================================================================================= "
Remove-Item -Path libvisionprotocol.dll -Force
Remove-Item -Path libvisionprotocol.lib -Force
Copy-Item -Path ..\build\windows-x86_64\libvisionprotocol.dll -Destination "." -Force
Copy-Item -Path ..\build\windows-x86_64\libvisionprotocol.lib -Destination "." -Force

# cd ..
Write-Host "========================================================================================= "
Write-Host "Build visionprotocol.cp38-win_amd64.pyd"
Write-Host "========================================================================================= "

C:\project\.pyvenv38\Scripts\activate.ps1
python .\setup_windows.py build_ext --inplace --plat-name win-amd64
deactivate

Write-Host "========================================================================================= "
Write-Host "Build visionprotocol.cp39-win_amd64.pyd"
Write-Host "========================================================================================= "

C:\project\.pyvenv39\Scripts\activate.ps1
python .\setup_windows.py build_ext --inplace --plat-name win-amd64
deactivate

Write-Host "========================================================================================= "
Write-Host "Build visionprotocol.cp310-win_amd64.pyd"
Write-Host "========================================================================================= "

C:\project\.pyvenv310\Scripts\activate.ps1
python .\setup_windows.py build_ext --inplace --plat-name win-amd64
deactivate

Write-Host "========================================================================================= "
Write-Host "Build visionprotocol.cp311-win_amd64.pyd"
Write-Host "========================================================================================= "

C:\project\.pyvenv311\Scripts\activate.ps1
python .\setup_windows.py build_ext --inplace --plat-name win-amd64
deactivate

Write-Host "========================================================================================= "
Write-Host "Build visionprotocol.cp312-win_amd64.pyd"
Write-Host "========================================================================================= "

C:\project\.pyvenv312\Scripts\activate.ps1
python .\setup_windows.py build_ext --inplace --plat-name win-amd64
deactivate

# Write-Host "========================================================================================= "
# Write-Host "Build visionprotocol.cp313-win_amd64.pyd"
# Write-Host "========================================================================================= "

# C:\project\.pyvenv313\Scripts\activate.ps1
# python .\setup_windows.py build_ext --inplace --plat-name win-amd64
# deactivate