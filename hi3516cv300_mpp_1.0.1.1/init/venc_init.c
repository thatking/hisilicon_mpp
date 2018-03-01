#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include "hi_type.h"

extern int VENC_ModInit(void);
extern void VENC_ModExit(void);


extern HI_U32 VencBufferCache;
module_param( VencBufferCache, uint, S_IRUGO);

extern HI_U32 FrameBufRecycle;
module_param( FrameBufRecycle, uint, S_IRUGO);


EXPORT_SYMBOL(VencBufferCache);
EXPORT_SYMBOL(FrameBufRecycle);

static int __init venc_mod_init(void){
	VENC_ModInit();
	return 0;
}
static void __exit venc_mod_exit(void){
	VENC_ModExit();
}

module_init(venc_mod_init);
module_exit(venc_mod_exit);

MODULE_LICENSE("Proprietary");




