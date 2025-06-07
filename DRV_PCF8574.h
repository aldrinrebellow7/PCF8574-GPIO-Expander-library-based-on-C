/*********************************.FILE_HEADER.*******************************
                               <Copyright Notice>
.File          :drv_PCF8574.h
.Summary       :header file of PCF8574 driver
.Note          :

 Author        Date              Description
------------------------------------------------------------------------------
<Author name>   <DDMMMYYYY>       <changes made>
 AFIL			26-04-2025		 Created driver

******************************************************************************/

#ifndef DRV_PCF8574_H_
#define DRV_PCF8574_H_

/************************** defines ******************************************/

/********************** typedef enums *****************************************/
#define ADDRESS_I2C_PCF8574	(0x20)
#define PCF8574_MAX_CHANNEL	(8U)
#define PCF8574_GPIO_RESET_STATE (0xFF)
/* I2c transfer types */
typedef enum
{
	I2C_NORMAL = 0U,
	I2C_INTERUPT,
	I2C_DMA
}I2C_TRANSFER_TYPE;

typedef enum
{
	GPX_PIN_MODE_OUTPUT = 0U,
	GPX_PIN_MODE_INPUT = 1U,
}PCF8547_PIN_MODE;

/* Current operation  */
typedef enum
{
	IO_IDLE = 0U,
    IO_READ,
    IO_WRITE,
    IO_TOGGLE,
	IO_INTERRUPT
} PCF8574_Operation;

/* function return types */
typedef enum
{
    RETURN_ERROR 	= 0U,
	RETURN_SUCCESS
}ReturnType;


typedef enum
{
    PIN0 = 0U,
    PIN1,
    PIN2,
    PIN3,
    PIN4,
    PIN5,
    PIN6,
    PIN7,
	ALL_PINS
} PCF8574_PinType;

/**************************** typedef structures ********************************/

/* Flag status structure */
typedef struct
{
	uint8_t ReadStatus_Flag 		: 1;
	uint8_t WriteStatus_Flag 		: 1;
	uint8_t ToggleStatus_Flag 		: 1;
	uint8_t InterruptStatus_Flag 	: 1;

	uint8_t reserved_flags			: 4;
}PCF8574_FlagStatus_st;

/* Pin state structure */
typedef union
{
    struct
    {
    	uint8_t P0 : 1;  // Pin 0
    	uint8_t P1 : 1;  // Pin 1
    	uint8_t P2 : 1;  // Pin 2
    	uint8_t P3 : 1;  // Pin 3
    	uint8_t P4 : 1;  // Pin 4
    	uint8_t P5 : 1;  // Pin 5
    	uint8_t P6 : 1;  // Pin 6
    	uint8_t P7 : 1;  // Pin 7
    } bit;
    uint8_t all;        // Access all 8 bits directly
} PCF8574_PinState_st;

/* Pin mode structure */
typedef union
{
    struct
    {
    	uint8_t P0 : 1;  // Pin 0
    	uint8_t P1 : 1;  // Pin 1
    	uint8_t P2 : 1;  // Pin 2
    	uint8_t P3 : 1;  // Pin 3
    	uint8_t P4 : 1;  // Pin 4
    	uint8_t P5 : 1;  // Pin 5
    	uint8_t P6 : 1;  // Pin 6
    	uint8_t P7 : 1;  // Pin 7
    } bit;
    uint8_t all;        // Access all 8 bits directly
} PCF8574_PinMode_st;


typedef struct
{
    PCF8574_FlagStatus_st 	flags;
    PCF8574_PinState_st 	pinState;  /* Output register (with bits and full byte access)*/
    PCF8574_PinMode_st  	pinMode;
    PCF8574_Operation		currOperation;
    uint8_t 				dev_address;         /* I2c Address */


} PCF8574_HandleType;

/**************************** functions ***************************************/
/* PCF8574 related functions */
ReturnType PCF8574_Init(PCF8574_HandleType *hpcf,uint8_t device_address);

ReturnType PCF8574_Read(PCF8574_HandleType *hpcf,
						PCF8574_PinType pin,
						I2C_TRANSFER_TYPE mode);
ReturnType PCF8574_Write(PCF8574_HandleType *hpcf,
						PCF8574_PinType pin,
						uint8_t* data,
						I2C_TRANSFER_TYPE mode);
ReturnType PCF8574_Toggle(PCF8574_HandleType *hpcf,
							PCF8574_PinType pin,
							I2C_TRANSFER_TYPE mode);
PCF8574_Operation PCF8574_GetOpStatus(PCF8574_HandleType *hpcf);
ReturnType PCF8574_GetPinState(PCF8574_HandleType *hpcf, uint8_t* state);
ReturnType PCF8574_GetFlagStatus(PCF8574_HandleType *hpcf,
								PCF8574_Operation op,
								uint8_t* status);
ReturnType PCF8574_ClearFlagStatus(PCF8574_HandleType *hpcf,
								PCF8574_Operation op);
ReturnType PCF8574_SetPinMode(PCF8574_HandleType *hpcf,
		PCF8574_PinType pin,
		PCF8547_PIN_MODE mode);
void Callback_IRQ_INT_Pin(void);
void Callback_PCF8574TxComplete(void);
void Callback_PCF8574RxComplete(void);
#endif /* DRV_PCF8574_H_ */
