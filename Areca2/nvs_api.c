#include "gd32f30x.h" // Include GD32F303 standard peripheral library
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
//#include "esp_log.h"
//#include "esp_console.h"
//#include "argtable3/argtable3.h"
//#include "freertos/FreeRTOS.h"
//#include "freertos/event_groups.h"
//#include "esp_err.h"
#include "nvs.h"
#include "nvs_api.h"




typedef enum
{
  false = 0,
  true
} bool;

typedef enum {
    NVS_TYPE_U8,   
    NVS_TYPE_I8,   
    NVS_TYPE_U16,  
    NVS_TYPE_I16,  
    NVS_TYPE_U32,  
    NVS_TYPE_I32, 
    NVS_TYPE_U64,  
    NVS_TYPE_I64, 
    NVS_TYPE_STR,  
    NVS_TYPE_BLOB, 
    NVS_TYPE_ANY 
} nvs_type_t;

typedef struct {
    nvs_type_t Type;
    const char *pStr;
}TypeStrPairT;

static const TypeStrPairT TypeStrPair[] = {
    { NVS_TYPE_U8, "u8" },
    { NVS_TYPE_I8, "i8" },
    { NVS_TYPE_U16, "u16" },
    { NVS_TYPE_I16, "i16" },
    { NVS_TYPE_U32, "u32" },
    { NVS_TYPE_I32, "i32" },
    { NVS_TYPE_U64, "u64" },
    { NVS_TYPE_I64, "i64" },
    { NVS_TYPE_STR, "str" },
    { NVS_TYPE_BLOB, "blob" },
    { NVS_TYPE_ANY, "any" },
};

typedef enum {
    ESP_OK = 0,             // Success
    // ... (Many other error codes representing specific conditions)
    ESP_FAIL = -1,          // Generic failure
} esp_err_t;



static const size_t TYPE_STR_PAIR_SIZE = sizeof(TypeStrPair) / sizeof(TypeStrPair[0]);
static char CurrentNamespace[16] = "storage";
static const char *TAG = "cmd_nvs";

static nvs_type_t StrToType(const char *pType) 
{

    for (int i = 0; i < TYPE_STR_PAIR_SIZE; i++) 
	{
        const TypeStrPairT *p = &TypeStrPair[i];
        if (strncmp(pType, p->pStr, strlen(p->pStr)) == 0) 
		{
            return  p->Type;
        }
    }

    return NVS_TYPE_ANY;
}

//esp_err_t StoreBlob(nvs_handle Nvs, const char *pKey, const char *pStrValues) 
esp_err_t StoreBlob(const char *pKey, const char *pStrValues) 
{
    uint8_t Value = 0;
    size_t StrLen = strlen(pStrValues);
    size_t BlobLen = StrLen / 2;

    if (StrLen % 2) 
	{
//        ESP_LOGE(TAG, "Blob data must contain even number of characters");
        return ESP_FAIL;
    }

    char *pBlob = (char *)malloc(BlobLen);
    if (pBlob == NULL) 
	{
        return ESP_FAIL;
    }

    for (int i = 0, j = 0; i < StrLen; i++) 
	{
        char ch = pStrValues[i];
        if (ch >= '0' && ch <= '9') 
		{
            Value = ch - '0';
        } 
		else if (ch >= 'A' && ch <= 'F') 
		{
            Value = ch - 'A' + 10;
        }
		else if (ch >= 'a' && ch <= 'f') 
		{
            Value = ch - 'a' + 10;
        } 
		else 
		{
//            ESP_LOGE(TAG, "Blob data contain invalid character");
            free(pBlob);
            return ESP_FAIL;
        }

        if (i & 1) 
		{
            pBlob[j++] += Value;
        } 
		else 
		{
            pBlob[j] = Value << 4;
        }
    }

//    esp_err_t err = ESP_OK

    return ESP_OK;
}

void PrintBlob(const char *pBlob, size_t Len) 
{
	for (int i = 0; i < Len; i++) 
	{
		printf("%02x", pBlob[i]);
	}
	printf("\n");
}
//int16_t SetValue(const char *pKey, const char *pStrType, const char *pStrValue);

//int16_t SetValue(const char *pKey, const char *pStrType, const char *pStrValue) 
//{
//	esp_err_t Err;
////	nvs_handle Nvs;
//	bool RangeError = false;
//
//	nvs_type_t Type = StrToType(pStrType);
//
//	if ((Type == NVS_TYPE_ANY) || (pStrValue == NULL))
//	{
//		return ESP_FAIL;
//	}
//
//	
////    Err = nvs_open(CurrentNamespace, NVS_READWRITE, &Nvs);
//	if (Err != ESP_OK) 
//	{
//		return Err;
//	}
//
//	if (Type == NVS_TYPE_I8) 
//	{
//		int32_t Value = strtol(pStrValue, NULL, 0);
//		if (Value < INT8_MIN || Value > INT8_MAX || errno == ERANGE) 
//		{
//			RangeError = true;
//		} 
//		else 
//		{
////			Err = nvs_set_i8(Nvs, pKey, (int8_t)Value);
//		}
//	}
//	else if (Type == NVS_TYPE_U8) 
//	{
//		uint32_t Value = strtoul(pStrValue, NULL, 0);
//		if (Value > UINT8_MAX || errno == ERANGE) 
//		{
//			RangeError = true;
//		} 
//		else 
//		{
////			Err = nvs_set_u8(Nvs, pKey, (uint8_t)Value);
//		}
//	}
//	else if (Type == NVS_TYPE_I16) 
//	{
//		int32_t Value = strtol(pStrValue, NULL, 0);
//		if (Value < INT16_MIN || Value > INT16_MAX || errno == ERANGE) 
//		{
//			RangeError = true;
//		} 
//		else 
//		{
////			Err = nvs_set_i16(Nvs, pKey, (int16_t)Value);
//		}
//	} 
//	else if (Type == NVS_TYPE_U16) 
//	{
//		uint32_t Value = strtoul(pStrValue, NULL, 0);
//		if (Value > UINT16_MAX || errno == ERANGE) 
//		{
//			RangeError = true;
//		}
//		else 
//		{
////			Err = nvs_set_u16(Nvs, pKey, (uint16_t)Value);
//		}
//	}
//	else if (Type == NVS_TYPE_I32) 
//	{
//		int32_t Value = strtol(pStrValue, NULL, 0);
//		if (errno != ERANGE) 
//		{
////			Err = nvs_set_i32(Nvs, pKey, Value);
//		}
//	}
//	else if (Type == NVS_TYPE_U32) 
//	{
//		uint32_t Value = strtoul(pStrValue, NULL, 0);
//		if (errno != ERANGE) 
//		{
////			Err = nvs_set_u32(Nvs, pKey, Value);
//		}
//	} 
//	else if (Type == NVS_TYPE_I64) 
//	{
//		int64_t Value = strtoll(pStrValue, NULL, 0);
//		if (errno != ERANGE) 
//		{
////			Err = nvs_set_i64(Nvs, pKey, Value);
//		}
//	} 
//	else if (Type == NVS_TYPE_U64) 
//	{
//		uint64_t Value = strtoull(pStrValue, NULL, 0);
//		if (errno != ERANGE) 
//		{
////			Err = nvs_set_u64(Nvs, pKey, Value);
//		}
//	} 
//	else if (Type == NVS_TYPE_STR) 
//	{
////		Err = nvs_set_str(Nvs, pKey, pStrValue);
//	} 
//	else if (Type == NVS_TYPE_BLOB) 
//	{
////		Err = StoreBlob(Nvs, pKey, pStrValue);
//	}
//
//	if (RangeError || errno == ERANGE) 
//	{
////		nvs_close(Nvs);
////		return ESP_ERR_NVS_VALUE_TOO_LONG;
//	}
//
//	if (Err == ESP_OK) 
//	{
////		Err = nvs_commit(Nvs);
////		if (Err == ESP_OK) 
////		{
////			ESP_LOGI(TAG, "Value stored under key '%s'", pKey);
//		}
//	}
//
////	nvs_close(Nvs);
//	return ESP_OK;
//}

int16_t SetValueU8(const char *pKey, unsigned char Data) 
{
	int16_t Err;
//	nvs_handle Nvs;
	bool RangeError = false;

	if (pKey == NULL)
	{
		return ESP_FAIL;
	}

//	Err = nvs_open(CurrentNamespace, NVS_READWRITE, &Nvs);
	if (Err != ESP_OK) 
	{
		return Err;
	}

//	Err = nvs_set_u8(Nvs, pKey, Data);

	if (RangeError || errno == ERANGE) 
	{
//		nvs_close(Nvs);
		return ESP_FAIL;
	}

	if (Err == ESP_OK) 
	{
//		Err = nvs_commit(Nvs);
		if (Err == ESP_OK) 
		{
//			ESP_LOGI(TAG, "Value stored under key '%s'", pKey);
		}
	}

//	nvs_close(Nvs);
	return Err;
}

int16_t SetValueU32(const char *pKey, unsigned int Data) 
{
	int16_t Err;
//	nvs_handle Nvs;
	bool RangeError = false;

	if (pKey == NULL)
	{
		return ESP_FAIL;
	}

//	Err = nvs_open(CurrentNamespace, NVS_READWRITE, &Nvs);
	if (Err != ESP_OK) 
	{
		return Err;
	}

	if (Data > UINT32_MAX )
	{
		RangeError = true;
	} 
	else 
	{
//		Err = nvs_set_u32(Nvs, pKey, Data);
	}

	if (RangeError || errno == ERANGE) 
	{
//		nvs_close(Nvs);
		return ESP_FAIL;
	}

	if (Err == ESP_OK) 
	{
//		Err = nvs_commit(Nvs);
		if (Err == ESP_OK) 
		{
//			ESP_LOGI(TAG, "Value stored under key '%s'", pKey);
		}
	}

//	nvs_close(Nvs);
	return Err;
}

int16_t SetValueStr(const char *pKey, char *pData) 
{
	int16_t Err;
//	nvs_handle Nvs;
	bool RangeError = false;

	if ((pKey == NULL) || (pData == NULL)) 
	{
		return ESP_FAIL;
	}

//	Err = nvs_open(CurrentNamespace, NVS_READWRITE, &Nvs);
	if (Err != ESP_OK) 
	{
		return Err;
	}

//	Err = nvs_set_str(Nvs, pKey, pData);

	if (RangeError || errno == ERANGE) 
	{
//		nvs_close(Nvs);
		return ESP_FAIL;
	}

	if (Err == ESP_OK) 
	{
//		Err = nvs_commit(Nvs);
		if (Err == ESP_OK) 
		{
//			ESP_LOGI(TAG, "Value stored under key '%s'", pKey);
		}
	}

//	nvs_close(Nvs);

	return Err;
}

int16_t GetValue(const char *pKey, const char *pStrType) 
{
//	nvs_handle Nvs;
	int16_t Err;

	nvs_type_t Type = StrToType(pStrType);

	if (Type == NVS_TYPE_ANY) 
	{
		return ESP_FAIL;
	}

//	Err = nvs_open(CurrentNamespace, NVS_READONLY, &Nvs);
	if (Err != ESP_OK) 
	{
		return Err;
	}

	if (Type == NVS_TYPE_I8) 
	{
		int8_t Value;
//		Err = nvs_get_i8(Nvs, pKey, &Value);
		if (Err == ESP_OK) 
		{
			printf("Value associated with key '%s' is %d \n", pKey, Value);
		}
	} 
	else if (Type == NVS_TYPE_U8) 
	{
		uint8_t Value;
//		Err = nvs_get_u8(Nvs, pKey, &Value);
		if (Err == ESP_OK) 
		{
			printf("Value associated with key '%s' is %u \n", pKey, Value);
		}
	} 
	else if (Type == NVS_TYPE_I16) 
	{
		int16_t Value;
//		Err = nvs_get_i16(Nvs, pKey, &Value);
		if (Err == ESP_OK) 
		{
			printf("Value associated with key '%s' is %d \n", pKey, Value);
		}
	} 
	else if (Type == NVS_TYPE_U16) 
	{
		uint16_t Value;
//		if ((Err = nvs_get_u16(Nvs, pKey, &Value)) == ESP_OK) 
//		{
//			printf("Value associated with key '%s' is %u", pKey, Value);
//		}
	} 
	else if (Type == NVS_TYPE_I32) 
	{
		int32_t Value;
//		if ((Err = nvs_get_i32(Nvs, pKey, &Value)) == ESP_OK) 
//		{
//			printf("Value associated with key '%s' is %d \n", pKey, Value);
//		}
	} 
	else if (Type == NVS_TYPE_U32) 
	{
		uint32_t Value;
//		if ((Err = nvs_get_u32(Nvs, pKey, &Value)) == ESP_OK) 
//		{
//			printf("Value associated with key '%s' is %u \n", pKey, Value);
//		}
	} 
	else if (Type == NVS_TYPE_I64) 
	{
		int64_t Value;
//		if ((Err = nvs_get_i64(Nvs, pKey, &Value)) == ESP_OK) 
//		{
//			printf("Value associated with key '%s' is %lld \n", pKey, Value);
//		}
	} 
	else if (Type == NVS_TYPE_U64) 
	{
		uint64_t Value;
//		if ( (Err = nvs_get_u64(Nvs, pKey, &Value)) == ESP_OK) 
//		{
//			printf("Value associated with key '%s' is %llu \n", pKey, Value);
//		}
	} 
	else if (Type == NVS_TYPE_STR) 
	{
		size_t Len;
//		if ( (Err = nvs_get_str(Nvs, pKey, NULL, &Len)) == ESP_OK) 
//		{
//			char *pStr = (char *)malloc(Len);
//			if ( (Err = nvs_get_str(Nvs, pKey, pStr, &Len)) == ESP_OK) 
//			{
//				printf("String associated with key '%s' is %s \n", pKey, pStr);
//			}
//			free(pStr);
//		}
	} 
	else if (Type == NVS_TYPE_BLOB) 
	{
		size_t Len;
//		if ( (Err = nvs_get_blob(Nvs, pKey, NULL, &Len)) == ESP_OK) 
//		{
//			char *pBlob = (char *)malloc(Len);
//			if ( (Err = nvs_get_blob(Nvs, pKey, pBlob, &Len)) == ESP_OK) 
//			{
//				printf("Blob associated with key '%s' is %d bytes long: \n", pKey, Len);
//				PrintBlob(pBlob, Len);
//			}
//			free(pBlob);
//		}
	}

//	nvs_close(Nvs);
	return Err;
}
// Function to read a byte from EEPROM
uint8_t eeprom_read_byte(uint16_t address) {
    uint8_t data;

    // Send start condition
    i2c_start_on_bus(I2C_SCL_GPIO_PORT);
    // Wait for start condition to be generated
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_SBSEND));

    // Send EEPROM address with write bit (0)
    i2c_master_addressing(I2C_SCL_GPIO_PORT, EEPROM_ADDRESS|0x01, I2C_TRANSMITTER);
    // Wait for address to be sent and ACK received
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_ADDSEND));
    // Clear ADDSEND flag
    i2c_flag_clear(I2C_SCL_GPIO_PORT, I2C_FLAG_ADDSEND);

    // Send high byte of EEPROM address
    i2c_data_transmit(I2C_SCL_GPIO_PORT, (address >> 8) & 0xFF);
    // Wait for data to be sent and ACK received
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_TBE));

    // Send low byte of EEPROM address
    i2c_data_transmit(I2C_SCL_GPIO_PORT, address & 0xFF);
    // Wait for data to be sent and ACK received
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_TBE));

    // Send repeated start condition
    i2c_start_on_bus(I2C_SCL_GPIO_PORT);
    // Wait for repeated start condition to be generated
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_SBSEND));

    // Send EEPROM address with read bit (1)
    i2c_master_addressing(I2C_SCL_GPIO_PORT, EEPROM_ADDRESS|0x01, I2C_RECEIVER);
    // Wait for address to be sent and ACK received
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_ADDSEND));
    // Clear ADDSEND flag
    i2c_flag_clear(I2C_SCL_GPIO_PORT, I2C_FLAG_ADDSEND);

    // Disable ACK before reading last byte
    i2c_ack_config(I2C_SCL_GPIO_PORT, I2C_ACK_DISABLE);

    // Wait for data to be received
    while (!i2c_flag_get(I2C_SCL_GPIO_PORT, I2C_FLAG_RBNE));
    // Read data from I2C data register
    data = i2c_data_receive(I2C_SCL_GPIO_PORT);

    // Send stop condition
    i2c_stop_on_bus(I2C_SCL_GPIO_PORT);
    // Wait for stop condition to be generated
    while (I2C_CTL0(I2C_SCL_GPIO_PORT) & I2C_CTL0_STOP);

    return data;
}

int16_t GetValueU8(const char *pKey, unsigned char *pData) 
{
//	nvs_handle Nvs;
	esp_err_t Err;
	uint8_t Value;

	if(pData == NULL) { 
		return ESP_FAIL;
	}

//    eeprom_read_byte(0x01);
//	Err = nvs_open(CurrentNamespace, NVS_READONLY, &Nvs);
//	if (Err != ESP_OK) {
//		return Err;
//	}

//	Err = nvs_get_u8(Nvs, pKey, &Value);
//	if (Err == ESP_OK) {
//		printf("Value associated with key '%s' is %u \n", pKey, Value);
//		*pData = Value;
//	}
//
//    nvs_close(Nvs);
    return Err;
}

int16_t GetValueU32(const char *pKey, unsigned int *pData) 
{
//	nvs_handle Nvs;
	esp_err_t Err;
//	uint32_t Value;
//
//	if(pData == NULL) { 
//		return ESP_ERR_NVS_INVALID_STATE;
//	}
//
//	Err = nvs_open(CurrentNamespace, NVS_READONLY, &Nvs);
//	if (Err != ESP_OK) {
//		return Err;
//	}
//
//	Err = nvs_get_u32(Nvs, pKey, &Value);
//	if (Err == ESP_OK) {
//		printf("Value associated with key '%s' is %u \n", pKey, Value);
//		*pData = Value;
//	}
//
//	nvs_close(Nvs);
	return Err;
}

int16_t GetValueStr(const char *pKey, char *pData) 
{
//	nvs_handle Nvs;
	int16_t Err;
	size_t Len;

	if(pData == NULL) 
	{ 
		return ESP_FAIL;
	}

//	Err = nvs_open(CurrentNamespace, NVS_READONLY, &Nvs);
//	if (Err != ESP_OK) 
//	{
//		return Err;
//	}

//	if ((Err = nvs_get_str(Nvs, pKey, NULL, &Len)) == ESP_OK) 
//	{
//		char *pStr = (char *)malloc(Len);
//		if ( (Err = nvs_get_str(Nvs, pKey, pStr, &Len)) == ESP_OK) {
//			printf("String associated with key '%s' is %s \n", pKey, pStr);
//			strncpy(pData, pStr, Len);
//		}
//
//		free(pStr);
//	}
//
//	nvs_close(Nvs);
	return Err;
}

int16_t Erase(const char *pKey) 
{
//	nvs_handle Nvs;
	int16_t Err;

//	Err = nvs_open(CurrentNamespace, NVS_READWRITE, &Nvs);
//
//	if (Err == ESP_OK) 
//	{
//		Err = nvs_erase_key(Nvs, pKey);
//		if (Err == ESP_OK) 
//		{
//			Err = nvs_commit(Nvs);
//			if (Err == ESP_OK) 
//			{
//				ESP_LOGI(TAG, "Value with key '%s' erased", pKey);
//			}
//		}
//
//		nvs_close(Nvs);
//	}

	return Err;
}

int16_t EraseAll(void) 
{
//	nvs_handle Nvs;

//	esp_err_t Err;

//	Err = nvs_open(CurrentNamespace, NVS_READWRITE, &Nvs);
//	if (Err == ESP_OK) 
//	{
//		Err = nvs_erase_all(Nvs);
//		if (Err == ESP_OK) 
//		{
//			Err = nvs_commit(Nvs);
//		}
//	}
//
//	ESP_LOGI(TAG, "Namespace '%s' was %s erased", CurrentNamespace, (Err == ESP_OK) ? "" : "not");
//	nvs_close(Nvs);
	return ESP_OK;
}

