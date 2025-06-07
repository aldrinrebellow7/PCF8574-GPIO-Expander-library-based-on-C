/*********************************.FILE_HEADER.*******************************
                               <Copyright Notice>
.File          :drv_PCF8574.c
.Summary       :source file of PCF8574 driver
.Note          :

 Author        Date              Description
------------------------------------------------------------------------------
<Author name>   <DDMMMYYYY>       <changes made>
 AFIL			26-04-2025		 Created driver

 ******************************************************************************/

/**************************** includes **************************************/
#include "main.h"
#include "DRV_PCF8574.h"

/**************************** defines ***************************************/
#define I2C_INTERRUPT_MODE

#define PCF8574_I2C_DELAY		1000U
#define PCF8574_I2C_DATA_SIZE	1U		/* 8bit*/

#define ALL_PIN_INPUT	255U
#define ALL_PIN_OUTPUT	0U

#define TOGGLE_BIT(REG, BIT)   ((REG) ^= (BIT))
PCF8574_HandleType *p_HpcfHandle = NULL;
/************************* typedefs ****************************************/


/************************* function prototypes *****************************/
/* Controller specific functions */
ReturnType i2c_init(uint8_t address);
ReturnType i2c_read(uint8_t address, uint8_t* pData, I2C_TRANSFER_TYPE mode);
ReturnType i2c_write(uint8_t address, uint8_t* pData, I2C_TRANSFER_TYPE mode);

/* interrupt Callback functions of STM */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
/************************* function definitions ****************************/
/**************************.PCF8574_Init().**********************************
 .Purpose        : Initialization of PCF8574 handler
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType PCF8574_Init(PCF8574_HandleType *hpcf,
		uint8_t device_address)
{
	ReturnType ret = RETURN_ERROR;
	p_HpcfHandle = hpcf;
	/* Initialize the i2c device */
	if(RETURN_SUCCESS == i2c_init(device_address))
	{
		/* Set the device address */
		hpcf->dev_address 	= device_address;

		ret = RETURN_SUCCESS;
	}
	return ret;
}

/***********************.PCF8574_Read().*************************************
 .Purpose        : Update the status of IO pins
 .Returns        : RETURN_ERROR
				   RETURN_SUCCESS
 .Note           : Check PCF8574_PinState_st for pin states
 	 	 	 	   Check PCF8574_FlagStatus_st for operation status
 ****************************************************************************/
ReturnType PCF8574_Read(PCF8574_HandleType *hpcf,
		PCF8574_PinType pin,
		I2C_TRANSFER_TYPE mode)
{
	ReturnType ret = RETURN_ERROR;

	/* Check if handler initialized */
	if(hpcf != NULL)
	{
		/* perform i2c read based on the type */
		if(RETURN_SUCCESS == i2c_read(hpcf->dev_address,&(hpcf->pinState.all), mode))
		{
			/* Set current operation as read */
			hpcf->currOperation = IO_READ;

			ret = RETURN_SUCCESS;
		}
	}
	/* Return status */
	return ret;
}

/***********************.PCF8574_Write().*************************************
 .Purpose        : Change the state of the IO pin
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           : Check PCF8574_FlagStatus_st for operation status
 ****************************************************************************/
ReturnType PCF8574_Write(PCF8574_HandleType *hpcf,
		PCF8574_PinType pin,
		uint8_t* data,
		I2C_TRANSFER_TYPE mode)
{

	ReturnType ret 			= RETURN_ERROR;
	static uint8_t tempData = 0;

	tempData = hpcf->pinState.all;

	/* reset the operation status */
	hpcf->flags.WriteStatus_Flag = FALSE;


	if((hpcf != NULL) && (data != NULL))
	{
		/* whole 8 pin write */
		if(pin == ALL_PINS)
		{
			tempData = *data;
		}
		/* individual pin write */
		else
		{
			/* Set or clear only the specific bit */
			if (*data)
			{
				SET_BIT(tempData, (1 << pin));
			}
			else
			{
				CLEAR_BIT(tempData, (1 << pin));
			}
		}

		/* Mask the input pins to avoid changing them */
		tempData = tempData | hpcf->pinMode.all;

		/* write the data through i2c */
		if(RETURN_SUCCESS == i2c_write(hpcf->dev_address, &tempData, mode))
		{
			/* Set current operation as write */
			hpcf->currOperation = IO_WRITE;
			/* update pin stte buffer */
			hpcf->pinState.all = tempData;
			ret = RETURN_SUCCESS;
		}
	}
	return ret;
}

/**********************.PCF8574_Toggle().*************************************
 .Purpose        : Toggle pins of the IO expander
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           : Check PCF8574_FlagStatus_st for operation status
 ****************************************************************************/
ReturnType PCF8574_Toggle(PCF8574_HandleType *hpcf,
		PCF8574_PinType pin,
		I2C_TRANSFER_TYPE mode)
{
	ReturnType ret 			= RETURN_ERROR;
	static uint8_t tempData = 0;
	tempData = hpcf->pinState.all;

	/* reset the operation status */
	hpcf->flags.ToggleStatus_Flag = FALSE;

	if(hpcf != NULL)
	{
		/* whole 8 pin toggle */
		if(pin == ALL_PINS)
		{
			tempData = ~(hpcf->pinState.all);
		}
		/* individual pin toggle */
		else
		{
			TOGGLE_BIT(tempData,(1 << pin));
		}

		/* Mask the input pins to avoid changing them */
		tempData = tempData | hpcf->pinMode.all;

		/* write the data through i2c */
		if(RETURN_SUCCESS == i2c_write(hpcf->dev_address, &tempData, mode))
		{
			/* update pin state buffer */
			hpcf->pinState.all = tempData;

			/* Set current operation as toggle */
			hpcf->currOperation = IO_TOGGLE;

			ret = RETURN_SUCCESS;
		}
	}
	return ret;
}
/*********************.PCF8574_GetPinState().********************************
 .Purpose        : Get the 8 bit IO state
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
PCF8574_Operation PCF8574_GetOpStatus(PCF8574_HandleType *hpcf)
{
	return (hpcf->currOperation);
}
/*********************.PCF8574_GetPinState().********************************
 .Purpose        : Get the 8 bit IO state
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType PCF8574_GetPinState(PCF8574_HandleType *hpcf, uint8_t* state)
{
	ReturnType ret = RETURN_ERROR;

	if(hpcf != NULL)
	{
		*state = hpcf->pinState.all;

		ret = RETURN_SUCCESS;
	}
	return ret;
}

/*******************.PCF8574_GetFlagStatus().********************************
 .Purpose        : Get the status of a flag
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType PCF8574_GetFlagStatus(PCF8574_HandleType *hpcf,
								PCF8574_Operation op,
								uint8_t* status)
{
	ReturnType ret = RETURN_ERROR;

	if(hpcf != NULL)
	{
		ret = RETURN_SUCCESS;
		/* Get the flag status from the structure based on operation */
		switch (op)
		{
		case IO_READ: 		*status = hpcf->flags.ReadStatus_Flag; 		break;
		case IO_WRITE: 		*status = hpcf->flags.WriteStatus_Flag; 	break;
		case IO_TOGGLE: 	*status = hpcf->flags.ToggleStatus_Flag; 	break;
		case IO_INTERRUPT: 	*status = hpcf->flags.InterruptStatus_Flag; break;
		default: 			ret = RETURN_ERROR; 				   		break;
		}
	}
	return ret;
}

/*********************.PCF8574_ClearFlagStatus().****************************
 .Purpose        : Clear the Status of a flag
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType PCF8574_ClearFlagStatus(PCF8574_HandleType *hpcf,
								PCF8574_Operation op)
{
	ReturnType ret = RETURN_ERROR;

	if(hpcf != NULL)
	{
		ret = RETURN_SUCCESS;
		/* Get the flag status from the structure based on operation */
		switch (op)
		{
		case IO_READ: 	hpcf->flags.ReadStatus_Flag 		= FALSE; break;
		case IO_WRITE: 	hpcf->flags.WriteStatus_Flag 		= FALSE; break;
		case IO_TOGGLE: hpcf->flags.ToggleStatus_Flag		= FALSE; break;
		case IO_INTERRUPT: 	hpcf->flags.InterruptStatus_Flag= FALSE; break;
		default: 			ret = RETURN_ERROR; 				   	 break;
		}
	}
	return ret;
}

/******************************.i2c_read().**********************************
 .Purpose        : Controller Specific code for i2c receive
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType i2c_init(uint8_t address)
{
	ReturnType ret = RETURN_ERROR;

	/* Get the i2c handler */
	I2C_HandleTypeDef *pI2c = GetInstance_I2C1();

	/* Check if device is connected with correct address */
	if(HAL_OK == HAL_I2C_IsDeviceReady(pI2c,
			address << 1,
			10, 100))
	{
		ret = RETURN_SUCCESS;
	}

	return ret;
}
/******************************.i2c_read().**********************************
 .Purpose        : Controller Specific code for i2c receive
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType i2c_read(uint8_t address, uint8_t* pData, I2C_TRANSFER_TYPE mode)
{
	ReturnType ret = RETURN_ERROR;

	/* Get the i2c handler */
	I2C_HandleTypeDef *pI2c = GetInstance_I2C1();
	switch(mode)
	{
	// Normal mode
	case I2C_NORMAL:
	{
		if(HAL_OK == HAL_I2C_Master_Receive(pI2c,
				(address << 1),
				pData,
				PCF8574_I2C_DATA_SIZE,
				PCF8574_I2C_DELAY))
		{
			ret = RETURN_SUCCESS;
		}
		break;
	}
	// Interrupt mode
	case I2C_INTERUPT:
	{
		/* Check i2c state before operation */
		if(HAL_I2C_STATE_READY == HAL_I2C_GetState(pI2c))
		{
			if(HAL_OK == HAL_I2C_Master_Receive_IT(pI2c,
					(address << 1),
					pData,
					PCF8574_I2C_DATA_SIZE))
			{
				ret = RETURN_SUCCESS;
			}
		}
		break;
	}
	// DMA mode
	case I2C_DMA:
	{
		/* Check i2c state before operation */
		if(HAL_I2C_STATE_READY == HAL_I2C_GetState(pI2c))
		{
			/* abort any interrupt request */
			if(HAL_OK == HAL_I2C_Master_Receive_DMA(pI2c,
					(address << 1),
					pData,
					PCF8574_I2C_DATA_SIZE))
			{
				ret = RETURN_SUCCESS;
			}
		}
		break;
	}
	default:
		break;
	}

	return ret;
}

/*****************************.i2c_write().**********************************
 .Purpose        : Controller Specific code for i2c transmit
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType i2c_write(uint8_t address, uint8_t* pData, I2C_TRANSFER_TYPE mode)
{
	ReturnType ret = RETURN_ERROR;
	/* Controller specific Code */
	I2C_HandleTypeDef *pI2c = GetInstance_I2C1();
	switch(mode)
	{
	// Normal mode
	case I2C_NORMAL:
	{
		if(HAL_OK == (HAL_I2C_Master_Transmit(pI2c,
				(address << 1),
				pData,
				PCF8574_I2C_DATA_SIZE,
				PCF8574_I2C_DELAY)))
		{
			ret = RETURN_SUCCESS;
		}
		break;
	}
	// Interrupt mode
	case I2C_INTERUPT:
	{
		/* Check i2c state before operation */
		if(HAL_I2C_STATE_READY == HAL_I2C_GetState(pI2c))
		{
			if(HAL_OK == HAL_I2C_Master_Transmit_IT(pI2c,
					(address << 1),
					pData,
					PCF8574_I2C_DATA_SIZE))
			{
				ret = RETURN_SUCCESS;
			}
		}
		break;
	}
	// DMA mode
	case I2C_DMA:
	{
		/* Check i2c state before operation */
		if(HAL_I2C_STATE_READY == HAL_I2C_GetState(pI2c))
		{
			if(HAL_OK == HAL_I2C_Master_Transmit_DMA(pI2c,
					(address << 1),
					pData,
					PCF8574_I2C_DATA_SIZE))
			{
				ret = RETURN_SUCCESS;
			}
		}
		break;
	}
	default:
		break;
	}

	return ret;
}
/*********************.PCF8574_SetPinMode().********************************
 .Purpose        : Set the pin mode 0: output 1: input
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
ReturnType PCF8574_SetPinMode(PCF8574_HandleType *hpcf,
		PCF8574_PinType pin,
		PCF8547_PIN_MODE mode)
{
	ReturnType ret 	 = RETURN_ERROR;
	uint8_t tempData = 0;

	tempData = hpcf->pinMode.all;

	if(hpcf != NULL)
	{
		/* whole 8 pin toggle */
		if(pin == ALL_PINS)
		{
			/* Set or clear only the specific bit */
			if (GPX_PIN_MODE_INPUT == mode)
			{
				tempData = ALL_PIN_INPUT;
			}
			else
			{
				tempData = ALL_PIN_OUTPUT;
			}
		}
		/* individual pin toggle */
		else
		{
			/* Set or clear only the specific bit */
			if (GPX_PIN_MODE_INPUT == mode)
			{
				SET_BIT(tempData, (1 << pin));
			}
			else
			{
				CLEAR_BIT(tempData, (1 << pin));
			}
		}

		hpcf->pinMode.all = tempData;
	}


	return ret;
}
/*********************.HAL_I2C_MasterTxCpltCallback().************************
 .Purpose        : Callback for transmission complete for IT & DMA
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
inline void Callback_PCF8574TxComplete(void)
{
	p_HpcfHandle->currOperation = IO_IDLE;
	if(p_HpcfHandle->currOperation == IO_TOGGLE)
	{
		p_HpcfHandle->flags.ToggleStatus_Flag = TRUE;
	}
	else
	{
		p_HpcfHandle->flags.WriteStatus_Flag = TRUE;
		/* ISR for Write Start */
		/* ISR for Write End */
	}
}

/*********************.HAL_I2C_MasterRxCpltCallback().************************
 .Purpose        : Callback for reception complete for IT & DMA
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
inline void Callback_PCF8574RxComplete(void)
{
	p_HpcfHandle->flags.ReadStatus_Flag = TRUE;

	/* ISR for Read Start */
	p_HpcfHandle->currOperation = IO_IDLE;
	/* ISR for Read End */
}

/*********************.HAL_GPIO_EXTI_Callback().*****************************
 .Purpose        : Callback for GPIO interrupt Rising and falling
 .Returns        :  RETURN_ERROR
					RETURN_SUCCESS
 .Note           :
 ****************************************************************************/
inline void Callback_IRQ_INT_Pin(void)
{
	/* change  GetInstance_PFC1 () based on your pfc handler */
	p_HpcfHandle->flags.InterruptStatus_Flag = TRUE;
}
