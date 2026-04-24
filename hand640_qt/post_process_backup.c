
/*
    Openedges Enlight Network Compiler
    Classification main
*/

#include "stdint.h"
#include "enlight_network.h"

#ifdef __i386__
#   include <stdio.h>
#   define post_process_log(...) do {printf(__VA_ARGS__);} while(0)
#else
#   define post_process_log(...) do {_printf(__VA_ARGS__);} while(0)
#endif

extern void custom_postproc_init();

extern int custom_postproc_run(
    int num_output,
    enlight_act_tensor_t**  outputs           /**< output conf tensor number   */
    );


/** @brief Run custom post-processing
 *
 *      Unknonw post-processing
 *      
 *
 *  @param net_inst         instance Network instance.
 *  @param output_base      output tensor buffer base
 *  @param reserved 
 *
 *  @return Return number of output
 */
int run_post_process(
    void *net_inst,
    void *output_base,
    int  num_input,
    void *reserved)
{
    enlight_network_t *inst;
    enlight_custom_postproc_t* custom_param;

    int result;
    int i, num_output;
    enlight_act_tensor_t* output_tensors[MAX_NUM_OUTPUT];

    inst = (enlight_network_t *)net_inst;
    custom_param = (enlight_custom_postproc_t*)inst->post_proc_extension;

    num_output = enlight_custom_get_output_tensors(custom_param, output_tensors);

    for (i = 0; i < num_output; i++)
        output_tensors[i]->base = output_base;

    custom_postproc_init();

    result = custom_postproc_run(num_output, output_tensors);
    post_process_log("custom process result: %d\n", result);

    return num_output;
}
