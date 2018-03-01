/*     here is piris arch .
*
*
*  This file defines piris micro-definitions for user.
*
* History:
*     03-Mar-2016 Start of Hi351xx Digital Camera,H6
*
*/

#ifndef __PIRIS_HAL_H__
#define __PIRIS_HAL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


/***************base address***********************/
/* Piris use GPIO0_0, GPIO0_1, GPIO0_2, GPIO0_3 */
/* GPIO0's base address is 0x12140000 */

#define PIRISI_ADRESS_BASE     0x12140000        // Piris use GPIO0

#define  HI_IO_PIRISI_ADDRESS(base_va, x)  (base_va + ((x)-(PIRISI_ADRESS_BASE)))
#define  PIRIS_WRITE_REG(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define  PIRIS_READ_REG(Addr)         (*(volatile unsigned int *)(Addr))

#define PIRIS_CFG_REG(base_va)     HI_IO_PIRISI_ADDRESS(base_va, PIRISI_ADRESS_BASE + 0x003C)
#define PIRIS_CTRL_REG(base_va)     HI_IO_PIRISI_ADDRESS(base_va, PIRISI_ADRESS_BASE + 0x0400)


/***************PIRIS_DRV_Write REG value***********************/

#define PIRIS_HAL_PHASE0(base_va)                               \
    do                                                          \
    {                                                           \
        PIRIS_WRITE_REG(PIRIS_CTRL_REG(base_va), 0xF);          \
        PIRIS_WRITE_REG(PIRIS_CFG_REG(base_va) , 0x9);          \
    } while ( 0 );


#define PIRIS_HAL_PHASE1(base_va)                               \
    do                                                          \
    {                                                           \
        PIRIS_WRITE_REG(PIRIS_CTRL_REG(base_va), 0xF);          \
        PIRIS_WRITE_REG(PIRIS_CFG_REG(base_va) , 0x5);          \
    } while ( 0 );

#define PIRIS_HAL_PHASE2(base_va)                               \
    do                                                          \
    {                                                           \
        PIRIS_WRITE_REG(PIRIS_CTRL_REG(base_va), 0xF);          \
        PIRIS_WRITE_REG(PIRIS_CFG_REG(base_va) , 0x6);          \
    } while ( 0 );    
 
#define PIRIS_HAL_PHASE3(base_va)                               \
    do                                                          \
    {                                                           \
        PIRIS_WRITE_REG(PIRIS_CTRL_REG(base_va), 0xF);          \
        PIRIS_WRITE_REG(PIRIS_CFG_REG(base_va) , 0xA);          \
    } while ( 0 );                                      

    

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__PIRIS_HAL_H__*/




