/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  * 
  * 
  *	PROJEKT:   Stacja pogodowa o funkcjonalności:
  *		Wyświetlająca date i godzine
  *		Wyświetlająca bieżące parametry otoczenia
  *		Wyświetlająca dane pogodowe z internetu
  *		Wyświetlająca historię parametrów otoczenia z kilku ostatnich dni
  *		Ustawiająca panel fotowoltaiczny pod kątem największego natężenia światła
  *		Komunikacja z portem szeregowym komputera
  *		Sterowanie tym co jest wyświetlane za pomocą pilota IR
  *
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

#include <stdbool.h>
#include "ds18b20.h"
#include "lps25hb.h"
#include "ir.h"
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include <hagl_hal.h>
#include <hagl.h>
#include "channels.h"

#include "FUNCTIONS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUMBER_OF_MONTHS 12
#define MAX_MONTH_NAME_SIZE 20


// definicja do i2c do czujnika cisnienia
#define LPS25HB_ADDR			0xBA

#define LPS25HB_WHO_AM_I 		0x0F
#define LPS25HB_CTRL_REG1 		0x20
#define LPS25HB_CTRL_REG2 		0x21
#define LPS25HB_CTRL_REG3 		0x22
#define LPS25HB_CTRL_REG4 		0x23
#define LPS25HB_PRESS_OUT_XL 	0x28
#define LPS25HB_PRESS_OUT_L 	0x29
#define LPS25HB_PRESS_OUT_H 	0x2A
#define LPS25HB_TEMP_OUT_L 		0x2B
#define LPS25HB_TEMP_OUT_H 		0x2C

#define LPS25HB_CTRL_REG1_PD 	0x80
#define LPS25HB_CTRL_REG1_ODR2 	0x40
#define LPS25HB_CTRL_REG1_ODR1 	0x20
#define LPS25HB_CTRL_REG1_ODR0 	0x10
//koniec definicja do i2c do czujnika cisnienia
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/**
**Flaga: gdy 1 to pobierane sa dane z internetuprzez ESP   */
int pobierz_dane=0;
/**
** Flaga: gdy 1 to panel fotowoltaiczny jest ustawiany  */
int ustaw_panel=0;
/**
** Numer aktualnie wyświetlanego na ekranie TFT kanału  */
int licznik_kanalu=0;
/**
**  Flaga pozwalająca, aby dane były wyświetlane tylko raz na ekranie jeśli kanał jest nie zmieniany*/
int czy_wyswietlono=0;

int nowy_dzien=0;
int licznik_write_to_memory=1;
/**
**Struktura danych pozwalająca na łatwiejsze określenie czasu trwania sygnału impulsu z odbiornika IR   */
typedef enum {
	PULSE_9MS,
	PULSE_4MS,
	PULSE_2MS,
	PULSE_LONG,
	PULSE_SHORT,
	PULSE_ERROR,
}pulse_t;


uint8_t last_day;



/**
** Wszystkie zmienne RTC są związane z zegarem czasu rzeczywistego  */
RTC_DateTypeDef start_date; //date with which program starts
RTC_TimeTypeDef time;       //changed date
RTC_DateTypeDef date;       //changed time


/**
** Wszystkie zmienne uint są wykorzystywane jako licznik SysTick do różnych funkcji  */
uint time_compare;
uint start_Time;
uint start_Time2;
uint start_Time3;
uint start_Time4;
uint start_Time5;
uint start_Time6;
uint start_Time7;
uint start_Time11;
uint start_Time9;

uint start_Time12;

/**
**  Zmienne do obliczania temperatury  */
uint8_t temperature_calculated=0;
uint16_t temperature=0;
int next_day_flag=0;

int onlywrite=0;
int licznikdnia=0;
int czy_kolejny_dzien=0;
char months_names[NUMBER_OF_MONTHS][MAX_MONTH_NAME_SIZE] = //miesiace kalendarza
{ "styczen",
  "luty",
  "marzec",
  "kwiecien",
  "maj",
  "czerwiec",
  "lipiec",
  "sierpien",
  "wrzesien",
  "pazdziernik",
  "listopad",
  "grudzien",
};

int months_lenghts[NUMBER_OF_MONTHS] =     //dlugosc startowa poszczegolnych miesiecy
{ 31,
  28,
  31,
  30,
  31,
  30,
  31,
  31,
  30,
  31,
  30,
  31,
};

/**
** Zmienne result_memory odpowidają za wpisywanie danych do EEPROM  */
uint8_t result_memory = 0;
uint8_t result_memory2 = 0;
uint8_t result_memory3 = 0;
uint8_t result_memory4 = 0;







/**
** Struktura, która pozwala na tworzenie struktur miast, i przyporządkowywanie miastom danych pogodowych z internetu  */
struct Pogoda_w_miescie_ESP
{char temp[10];
char cisn[10];
char wiatr[10];
	};





/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/**
** To co robią poszczególne funkcje, opisywane jest przed blokami danych funkcji(np na końcu pliku main.c)  */
void initialize_main_calendar();
void set_new_date_and_time(RTC_DateTypeDef new_date,RTC_TimeTypeDef new_time, uint8_t newYear,uint8_t newMonth,uint8_t newDate,uint8_t newHours,uint8_t newMinutes,uint8_t newSeconds);
void next_day();
void is_pressure_sensor_connected();
void call_calendar();
void call_one_wire_temperature();
void print_lps25hb_sensor();
HAL_StatusTypeDef wire_reset();
uint8_t wire_read();
void write_to_memory();
void wire_write(uint8_t byte);
uint8_t byte_crc(uint8_t crc, uint8_t byte);
uint8_t wire_crc(const uint8_t* data, int len);

uint8_t lps_read_reg(uint8_t reg);
void lps_write_reg(uint8_t reg, uint8_t value);
void read_to_memory();
void line_append(uint8_t value);

void read_time_and_data();
void ESP_loop();

void ds18b20_temperature_sensor();
void print_what_you_write_and_entered();


void serial_port_information();

void start_GetTicks();

void serial_port_data();

void  lcd_put_in_while();

void ADC_measure();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
struct Pogoda_w_miescie_ESP Warszawa_pogoda;
struct Pogoda_w_miescie_ESP Gdansk_pogoda;
struct Pogoda_w_miescie_ESP Krakow_pogoda;


/**
  * @brief
  * Callback związany z odczytywaniem naciśniętego przycisku na pilocie IR oraz rozpoznawaniem go
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    switch (HAL_TIM_GetActiveChannel(&htim2))
    {
      case HAL_TIM_ACTIVE_CHANNEL_1:
        ir_tim_interrupt();
        break;
      default:
        break;
    }
  }
}

/**
  * @brief
  * Po naciśnięciu przycisku użytkownika, następuje reset czasu
  * @retval None
  */
bool is_button_pressed(void)
{
  if (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET) {
    return true;
  } else {
    return false;
  }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_TIM2_Init();
  MX_SPI2_Init();
  MX_UART5_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */




  	call_one_wire_temperature(); //sprawdzenie czy termometr podpiety



	initialize_main_calendar(start_date);  //data startowa po wlaczeniu programu
	call_calendar(); //wypisywanie miesiecy
	start_GetTicks(); //GetTicks
	  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); //włączanie PWM do obracania panelem fotowoltaicznym oraz jego kanałów
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	  htim1.Instance->CCR1=1300;
	  htim1.Instance->CCR2=1200;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	  is_pressure_sensor_connected();// czy czujnik cisnienia jest dolaczony
	  serial_port_information(); //informacja co wpisac do konsoli
	  lcd_init();//inicjalizacja wyświetlacza
	  ir_init(); //wystartowanie Input Capture timera

	  for(int i=0;i<=127;i++)//resetowanie pamięci EEPROM po każdorazowym restarcie
	  {
		  	  HAL_I2C_Mem_Write(&hi2c3, 0xA0, i, 1, 0, sizeof(0), 1000);
	  			    			  HAL_Delay(5);
	  }

	  last_day=date.Date;


  while (1)
  {
	  channel_of_display(); //sprawdzanie ktory kanal wyswietlacza
	  serial_port_data();   //pomiary na porcie szeregowym
	  print_what_you_write_and_entered(); //wypisanie na port szeregowy tego co wpisalismy
	  lcd_put_in_while(); //wyswietlanie poszczegolnych kanalow
	  write_to_memory(); //zapis do pamięci EEPROM danyh pogodowych
	  ESP_loop();  //pobieranie danych z internetu
	  ADC_measure(); //pomiar ADC dla obracania panelem fotowoltaicznym



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */









  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
/**
  * @brief
  * Wyświetlanie parametrów pogodowych na porcie szeregowym
  * @retval None
  */
void print_lps25hb_sensor()
{


	if(HAL_GetTick()-start_Time4>=5000)
			    	{printf("T = %.1f*C\n", lps25hb_read_temp());
	    printf("p = %.1f hPa\n", lps25hb_read_pressure());


			    		start_Time4=HAL_GetTick();
			    	}

}


/**
  * @brief
  * Sprawdzenie czy czujnik ciśnienia jest podłączony
  * @retval None
  */

void is_pressure_sensor_connected()
{
	printf("Searching...\n");
	if (lps25hb_init() == HAL_OK) {
	  printf("OK: LPS25HB\n");
	} else {
	  printf("Error: LPS25HB not found\n");
	  Error_Handler();
	}
	  if(HAL_GetTick()-start_Time3>=100)
		    	{


		    		start_Time3=HAL_GetTick();
		    	}
	}
/**
  * @brief
  * Wypisywanie nazw miesięcy na porcie szeregowym
  * @retval None
  */
void call_calendar()
{
	for(int i=0;i<=12;i++)
	  {printf("%c",months_names[2][i]);}
	printf("\n");



}
/**
  * @brief
  * Sprawdzenie czy czujnik temperatury jest podłączony
  * @retval None
  */
void  call_one_wire_temperature()
{
	 if (ds18b20_init() != HAL_OK) {
	    Error_Handler();
	  }

	  uint8_t ds1[DS18B20_ROM_CODE_SIZE];

	  if (ds18b20_read_address(ds1) != HAL_OK) {
	    Error_Handler();
	  }
}
/**
  * @brief
  * Inicjalizacja zegara RTC jakimiś danymi przy starcie systemu
  * @retval None
  */
void initialize_main_calendar(RTC_DateTypeDef start_date)
{
	start_date.Year = 25;
	start_date.Month = 06;
	start_date.Date = 1;
	start_date.WeekDay = RTC_WEEKDAY_TUESDAY;
	  HAL_RTC_SetDate(&hrtc, &start_date, RTC_FORMAT_BIN);



	}
/**
  * @brief
  * Ustawianie nowego czasu i daty na RTC
  * @retval None
  */
void set_new_date_and_time(RTC_DateTypeDef new_date,RTC_TimeTypeDef new_time, uint8_t newYear,uint8_t newMonth,uint8_t newDate,uint8_t newHours,uint8_t newMinutes,uint8_t newSeconds)
{

  			new_date.Year=newYear;
		  	new_date.Month=newMonth;
		  	new_date.Date=newDate;
		  	new_time.Hours = newHours;
		  	new_time.Minutes = newMinutes;
		  	new_time.Seconds = newSeconds;
		  	HAL_RTC_SetTime(&hrtc, &new_time, RTC_FORMAT_BIN);
		  	HAL_RTC_SetDate(&hrtc, &new_date, RTC_FORMAT_BIN);












}
/**
  * @brief
  * Odczyt daty i czasu z RTC
  * @retval None
  */
void read_time_and_data(uint time_compare)
{





			  // jeśli naciśnięto przycisk
			  if (is_button_pressed()) {
			  	// ważne: nieużywane pola muszą być wyzerowane
			  	RTC_TimeTypeDef new_time = {0};
			  	RTC_DateTypeDef new_date={0};

			  	// czekamy na zwolnienie przycisku
			  	while (is_button_pressed()) {}

			  	 set_new_date_and_time(new_date,new_time,20,4,15,20,30,22);

			  	// ustawiamy godzinę 07:45:00

			  }

			  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN); //pobieramy date i czas
			  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);


	if(time.Hours==0 && time.Seconds==0 && time.Minutes==0)  //nastepny dzien
			{
		if((HAL_GetTick()-time_compare)>1000)
		{RTC_TimeTypeDef new_time = {0};
	  	RTC_DateTypeDef new_date={0};

			next_day(new_date,new_time);
		time_compare=HAL_GetTick();
		}


			}










	if(HAL_GetTick()-start_Time>=2000)
	{


		printf("RTC: %04d-%02d-%02d, %02d:%02d:%02d\n", 2000 + date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);
		start_Time=HAL_GetTick();
	}

}

/**
  * @brief
  * Sprawdzenie czy następny dziń nastał
  * @retval None
  */
void next_day(RTC_DateTypeDef new_date,RTC_TimeTypeDef new_time)
{


	printf("nastepny dzien \n");

	if(months_lenghts[new_date.Month-1]==31)
	{
		if(new_date.Date<31)
		{
			new_date.Date++;
		}
		else
		{
			if(new_date.Month==12)
			{
				new_date.Month=1;
				new_date.Date=1;
				new_date.Year++;


			}
			else
			{
				new_date.Month++;
				new_date.Date=1;
			}
		}

	}
	else
	{
		if(months_lenghts[new_date.Month-1]==30)
		{
			if(new_date.Date<31)
					{
						new_date.Date++;
					}
			else
			{
				new_date.Month++;
			    new_date.Date=1;

			}

		}
		else
		{
			if(new_date.Month==2)
			{
				if(new_date.Date<28)
				{
					new_date.Date++;

				}
				else
				{
					if(new_date.Date==28)
					{

						if((new_date.Year/400)==0)
							{
								months_lenghts[1]=29;


							}
						else
						{

							if((new_date.Year/100)==0)
							{
								months_lenghts[1]=29;

							}
							else
							{
								if((new_date.Year/4)==0)
								{
									months_lenghts[1]=29;
								}

								else
								{
									months_lenghts[1]=28;
								}

							}

						}

					}

					else
					{

						if(new_date.Date==29)
						{
							new_date.Month++;
							new_date.Date=1;
						}
					}

				}
			}
		}

	}



	 HAL_RTC_GetTime(&hrtc, &new_time, RTC_FORMAT_BIN); //pobieramy date i czas
	 HAL_RTC_GetDate(&hrtc, &new_date, RTC_FORMAT_BIN);

	for(int i=0;i<=12;i++)
	  {


		printf("%c",months_names[new_date.Month-1][i]);
	  }
	printf("\n");


	}








/**
  * @brief
  * Odczyt danych z czujnika ciśnienia
  * @retval None
  */
uint8_t lps_read_reg(uint8_t reg)
{
	uint8_t value = 0;
	HAL_I2C_Mem_Read(&hi2c1, LPS25HB_ADDR, reg, 1, &value, sizeof(value), HAL_MAX_DELAY);

	return value;
}

void lps_write_reg(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c1, LPS25HB_ADDR, reg, 1, &value, sizeof(value), HAL_MAX_DELAY);
}
//koniec kod do czujnika cisnienia


/**
  * @brief
  * Zapis danych z czujnika temperatury
  * @retval None
  */
void ds18b20_temperature_sensor()
{
	  ds18b20_start_measure(NULL);
		    	 if(HAL_GetTick()-start_Time2>=2000)
		    		    	{
		    		 float temp = ds18b20_get_temp(NULL);
		    if (temp >= 80.0f)
		      printf("Sensor error...\n");
		    else
		      printf("temperature of ds18b20: %.1f\n", temp);






























		    start_Time2=HAL_GetTick();


		    		    	}


}




/**
  * @brief
  * Wypisanie na porcie szeregowym napisanego na porcie szeregowym tekstu
  * @retval None
  */
void print_what_you_write_and_entered()
{

	 uint8_t value;
		   			  if (HAL_UART_Receive(&huart2, &value, 1, 0) == HAL_OK)
		   				  line_append(value);
}




/**
  * @brief
  * Podstawowe informacie dotyczące pracy na porcie szeregowym
  * @retval None
  */
void serial_port_information()
{
	printf("jesli chcesz odczytywac pomiary na porcie szeregowym, napisz 'also reading' i zaakceptuj enterem \n");
	printf("jesli chcesz tylko pisac na porcie szeregowym, napisz 'just write' i zaakceptuj enterem \n");
}






/**
  * @brief
  * Wystartowanie timerów SYsticks do różnych funkcji
  * @retval None
  */
void start_GetTicks()
{
	time_compare =HAL_GetTick();
	 start_Time=HAL_GetTick();
	 start_Time2=HAL_GetTick();
	 start_Time3=HAL_GetTick();
	 start_Time4=HAL_GetTick();
	 start_Time5=HAL_GetTick();
	 start_Time6=HAL_GetTick();
	 start_Time7=HAL_GetTick();
	  start_Time11=HAL_GetTick();
	  start_Time9=HAL_GetTick();
	  start_Time12=HAL_GetTick();
}


void serial_port_data(){
	  if(onlywrite==0)
	  {
		  print_lps25hb_sensor();
	  // koniec czujnik cisnienia


// poczatek czujnik temperatury 1-wire
	    	ds18b20_temperature_sensor();


	  // koniec czujnik temperatury 1-wire



	    read_time_and_data(time_compare);
	  }
}

/**
  * @brief
  * Wyświetlanie na wyświetlaczu TFT poszczególnego tekstu na poszczególnych kanałach
  * @retval None
  */
void lcd_put_in_while(){


		 //lcd start

	 if(licznik_kanalu==-1)
			 {
				 if(czy_wyswietlono==0)
				 {wchar_t *result [20];
					 	 	 	 	 	 	 	 	 	 	 	 	 	 	  hagl_put_text((L"                        "), 20, 25, YELLOW, font6x9);
					 														  hagl_put_text((L"                        "), 20, 35, YELLOW, font6x9);
					 														  hagl_put_text((L"                        "), 20, 45, YELLOW, font6x9);
					 														  hagl_put_text((L"                        "), 20, 55, YELLOW, font6x9);
					 														  hagl_put_text((L"                        "), 20, 65, YELLOW, font6x9);
					 														  hagl_put_text((L"                        "), 20, 75, YELLOW, font6x9);
					 														  hagl_put_text((L"                        "), 20, 85, YELLOW, font6x9);
									  lcd_copy();

					  swprintf(result, 20,L"Temp Warsz: %s°C", Warszawa_pogoda.temp);
					  hagl_put_text((result), 20, 25, YELLOW, font6x9);
					  swprintf(result, 20,L"Wiatr Warsz:%skm/h", Warszawa_pogoda.wiatr);
					  hagl_put_text((result), 20, 35, YELLOW, font6x9);
					  swprintf(result, 20,L"Cisn Warsz: %s hPa", Warszawa_pogoda.cisn);
					  hagl_put_text((result), 20, 45, YELLOW, font6x9);
					  swprintf(result, 20,L"Temp Gdan: %s°C", Gdansk_pogoda.temp);
										  hagl_put_text((result), 20, 55, YELLOW, font6x9);
										  swprintf(result, 20,L"Wiatr Gdan:%skm/h", Gdansk_pogoda.wiatr);
										  hagl_put_text((result), 20, 65, YELLOW, font6x9);
										  swprintf(result, 20,L"Cisn Gdan: %s hPa", Gdansk_pogoda.cisn);
										  hagl_put_text((result), 20, 75, YELLOW, font6x9);

			  lcd_copy();
			  czy_wyswietlono=1;
				 }



			 }



	 if(licznik_kanalu==-2)
			 {



				 if(czy_wyswietlono==0)
				 {
	 	 	 	 	 	 hagl_put_text((L"                        "), 20, 25, YELLOW, font6x9);
					  hagl_put_text((L"                        "), 20, 35, YELLOW, font6x9);
					  hagl_put_text((L"                        "), 20, 45, YELLOW, font6x9);
					  hagl_put_text((L"                        "), 20, 55, YELLOW, font6x9);
					  hagl_put_text((L"                        "), 20, 65, YELLOW, font6x9);
					  hagl_put_text((L"                        "), 20, 75, YELLOW, font6x9);
					  hagl_put_text((L"                        "), 20, 85, YELLOW, font6x9);
									  lcd_copy();

									  wchar_t *result [20];

									  read_to_memory(0);
									  swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
									  hagl_put_text((result), 20, 25, YELLOW, font6x9);
									  read_to_memory(1);
									  swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
									  hagl_put_text((result), 20, 35, YELLOW, font6x9);
									  read_to_memory(2);
									 swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
									 hagl_put_text((result), 20, 45, YELLOW, font6x9);
									 read_to_memory(3);
									 swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
									 hagl_put_text((result), 20, 55, YELLOW, font6x9);
									 read_to_memory(4);
									swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
									hagl_put_text((result), 20, 65, YELLOW, font6x9);
									 read_to_memory(5);
									swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
									hagl_put_text((result), 20, 75, YELLOW, font6x9);
									 read_to_memory(6);
									swprintf(result, 20,L"temp:%d, %04d-%02d-%02d", result_memory,2000+result_memory2,result_memory3,result_memory4);
								    hagl_put_text((result), 20, 85, YELLOW, font6x9);



			  lcd_copy();
			  czy_wyswietlono=1;
				 }



			 }







		 if(licznik_kanalu==0)
		 {
			 if(czy_wyswietlono==0)
			 {
	 	 	 	 	 	  hagl_put_text((L"                        "), 20, 25, YELLOW, font6x9);
				  hagl_put_text((L"                        "), 20, 35, YELLOW, font6x9);
				  hagl_put_text((L"                        "), 20, 45, YELLOW, font6x9);
				  hagl_put_text((L"                        "), 20, 55, YELLOW, font6x9);
				  hagl_put_text((L"                        "), 20, 65, YELLOW, font6x9);
				  hagl_put_text((L"                        "), 20, 75, YELLOW, font6x9);
				  hagl_put_text((L"                        "), 20, 85, YELLOW, font6x9);
								  lcd_copy();
				  hagl_put_text((L"START"), 40, 55, YELLOW, font6x9);
		  lcd_copy();
		  czy_wyswietlono=1;
			 }



		 }
		 if(licznik_kanalu==1)
			 {

			 if(HAL_GetTick()-start_Time7>=1000)
			 			  					  		{  hagl_put_text((L"                 "), 40, 55, YELLOW, font6x9);
						 hagl_put_text((L"                 "), 40, 45, YELLOW, font6x9);
			  lcd_copy();


			  wchar_t *result [20];
							      swprintf(result, 20, L"%04d-%02d-%02d", 2000 + date.Year, date.Month, date.Date);
			  		   hagl_put_text((result), 40, 55, YELLOW, font6x9);

			  		 swprintf(result, 20, L"%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
			  					  		   hagl_put_text((result), 40, 45, YELLOW, font6x9);











			  		  lcd_copy();
			  		 czy_wyswietlono=1;
			 			  start_Time7=HAL_GetTick();					  		}

			 }
		 if(licznik_kanalu==2)
				 {

			 if(czy_wyswietlono==0 )
						 {
				    hagl_put_text((L"                  "), 40, 55, YELLOW, font6x9);
					 hagl_put_text((L"                 "), 40, 45, YELLOW, font6x9);
				  lcd_copy();

				  float temp = ds18b20_get_temp(NULL);
				  int whole = temp;
				  int remainder = (temp - whole) * 10;
				  wchar_t *result [20];
				      swprintf(result, 20, L"temp : %d.%d °C", whole,remainder);

				  		   hagl_put_text((result), 40, 55, YELLOW, font6x9);



				  		   float cisnienie=lps25hb_read_pressure();
				  		   whole = cisnienie;
				  		 remainder = (cisnienie - whole) * 10;
				  		 swprintf(result, 20, L"cisn : %d.%d °C", whole,remainder);
				  		 hagl_put_text((result), 40, 45, YELLOW, font6x9);



				  		  lcd_copy();




				  		 czy_wyswietlono=1;

						 }


				 }

		  //lcd koniec

}

/**
  * @brief
  * Zapis danych pogodowych do pamięci EEPROM
  * @retval None
  */
void write_to_memory()
{




			   // calculate_average_temperature(temperature);


			    if(HAL_GetTick()-start_Time6>=500000)
			    	{ licznik_write_to_memory++;

			    	float temp = ds18b20_get_temp(NULL);
			    	temperature_calculated=temperature_calculated+temp;
	  int8_t temperature=(temperature_calculated)/licznik_write_to_memory;

			    	//temperature=temp+temperature;
			    	//printf("dziala co 20sek: nowy dzien: %d \n",nowy_dzien);


			    	//uint8_t dzien = 0;

if(last_day==date.Date)
{	  						  HAL_I2C_Mem_Write(&hi2c3, 0xA0, (nowy_dzien*4)+0, 1, &temperature, sizeof(temperature), 1000);
			    			  HAL_Delay(10);
			  		    	  HAL_I2C_Mem_Write(&hi2c3, 0xA0, (nowy_dzien*4)+3, 1, &date.Date, sizeof(date.Date), 1000);
			    			  HAL_Delay(10);
			  		    	  HAL_I2C_Mem_Write(&hi2c3, 0xA0, (nowy_dzien*4)+2, 1, &date.Month, sizeof(date.Month), 1000);
			    			  HAL_Delay(10);
			  		    	  HAL_I2C_Mem_Write(&hi2c3, 0xA0, (nowy_dzien*4)+1, 1,&date.Year, sizeof(date.Year), 1000);

}
else{nowy_dzien++;
last_day=date.Date;
licznik_write_to_memory=0;

if(nowy_dzien>6)
{
	nowy_dzien=0;}
}




//			    	 HAL_I2C_Mem_Read(&hi2c3, 0xA0, 1*czy_kolejny_dzien, 1, &dzien, sizeof(dzien), 1000);




			 	  //HAL_Delay(5);

			  	 // HAL_I2C_Mem_Read(&hi2c3, 0xA0, 0x00, 1, &result, sizeof(result), 1000);


			    		start_Time6=HAL_GetTick();

			    }


}





/**
  * @brief
  * Odczyt danych z pamięci EEPROM
  * @retval None
  */

void read_to_memory(int value)
			    {
			    	 HAL_I2C_Mem_Read(&hi2c3, 0xA0, (value*4)+0, 1, &result_memory, sizeof(result_memory), 1000);
			    	 HAL_I2C_Mem_Read(&hi2c3, 0xA0, (value*4)+1, 1, &result_memory2, sizeof(result_memory2), 1000);
			    	 HAL_I2C_Mem_Read(&hi2c3, 0xA0, (value*4)+2, 1, &result_memory3, sizeof(result_memory3), 1000);
			    	 HAL_I2C_Mem_Read(&hi2c3, 0xA0, (value*4)+3, 1, &result_memory4, sizeof(result_memory4), 1000);
			    }

/**
  * @brief
  * Funkcja odpowiedzialna za odczyt danych pogodowych z internetu, w czym zawiera sie:
  * Sprawdzenie czy flaga została zmieniona, co czyni się poprzez wciśnięcie przycisku na pilocie
  * Podłączenie się do sieci WIFI przy pomocy ESP8266
  * Odczytanie danych z okreslonych w kodzie GET requestów
  * Wyswietlenie na ekranie TFT komunikatu o pobieraniu danych oraz o końcu pobierania
  *
  * @retval None
  */
void ESP_loop()
{

//
		if(pobierz_dane==1)
		{
			hagl_put_text(L"Pobieranie danych int", 40, 45, YELLOW, font6x9);
							  		  lcd_copy();
				  ESP_Init("Desktopx","CpWi6712");
				  ESP_GetData("T6OMO6R5MO94K4DT",Warszawa_pogoda.temp);
				  ESP_GetData("HK4BKPA2DEPEXD4C",Warszawa_pogoda.wiatr);
				  ESP_GetData("R26MG7BIUZLTGPY2",Warszawa_pogoda.cisn);
			//	  		ESP_GetData("7TUK80COZTUK4FVW",Gdansk_pogoda.temp);
			//	  		  ESP_GetData("JT5942AMN2X01J9L",Gdansk_pogoda.wiatr);
			//	  	ESP_GetData("G402BI4JAFWMNBLB",Gdansk_pogoda.cisn);
			//
			//	  	ESP_GetData("RQ61SUIA4I4O4GPQ",Krakow_pogoda.temp);
			//	  	  ESP_GetData("4XJCIGMRQZRS54WP",Krakow_pogoda.wiatr);
			//	  	ESP_GetData("IC5XORHNXMH36JDA",Krakow_pogoda.cisn);
				  hagl_put_text(L"Koniec pobierania danych", 40, 45, YELLOW, font6x9);
				  							  		  lcd_copy();
			pobierz_dane=0;
		}
		if(pobierz_dane==2)
				{
					hagl_put_text(L"Pobieranie danych int", 40, 45, YELLOW, font6x9);
									  		  lcd_copy();

						  		ESP_GetData("7TUK80COZTUK4FVW",Gdansk_pogoda.temp);
						  		  ESP_GetData("JT5942AMN2X01J9L",Gdansk_pogoda.wiatr);
						  	ESP_GetData("G402BI4JAFWMNBLB",Gdansk_pogoda.cisn);
					//
					//	  	ESP_GetData("RQ61SUIA4I4O4GPQ",Krakow_pogoda.temp);
					//	  	  ESP_GetData("4XJCIGMRQZRS54WP",Krakow_pogoda.wiatr);
					//	  	ESP_GetData("IC5XORHNXMH36JDA",Krakow_pogoda.cisn);
						  hagl_put_text(L"Koniec pobierania danych", 40, 45, YELLOW, font6x9);
						  							  		  lcd_copy();
					pobierz_dane=0;
				}





	}



/**
  * @brief
  * Funkcja odpowiedzialna za zmienianie położenia panelu fotowoltaicznego, w co zawiera sie:
  * Pomiar ADC napiecia na dzielnikach napiecia(czterech) skladajacych sie z fotorezystora oraz rezystora 10kOhm
  * Sparwdzenia warunkow, na ktory fotorezystor swiatlo swieci jak mocno
  * Obrot panelu fotowoltaicznego w dwoch osiach zgodnie z wczesniejszymi pomiarami
  * @retval None
  */
void ADC_measure()
{




	    	 	    	   if(HAL_GetTick()-start_Time12>=200)
	    	 	    	   					    	{
	    	 	    		  if(ustaw_panel==1)
	    	 	    		  		  {
	    	 	    		  	  uint32_t value_adc[4];

	    	 	    		  	    HAL_ADC_Start(&hadc1);
	    	 	    		  	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	    	 	    		  	    value_adc[0] = HAL_ADC_GetValue(&hadc1);

	    	 	    		  	    HAL_ADC_Start(&hadc1);
	    	 	    		  	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	    	 	    		  	    value_adc[1] = HAL_ADC_GetValue(&hadc1);

	    	 	    		  	    HAL_ADC_Start(&hadc1);
	    	 	    		  	    	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	    	 	    		  	    	    value_adc[2] = HAL_ADC_GetValue(&hadc1);

	    	 	    		  	    	    HAL_ADC_Start(&hadc1);
	    	 	    		  	    	 	    	    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	    	 	    		  	    	 	    	   value_adc[3] = HAL_ADC_GetValue(&hadc1);

	    	 	    	   printf("prawo gora=%lu, lewo gora=%lu\n", value_adc[2],value_adc[3]);
	   printf("lewo dol=%lu, prawo dol=%lu", value_adc[1], value_adc[0]);






	    	 	    	  if ( value_adc[3] >  value_adc[2]) {//topleft> topright
	    	 	    	  	    	htim1.Instance->CCR1=htim1.Instance->CCR1-10;//lewo
	    	 	    	  	     // delay(waittime);
	    	 	    	  	    }
	    	 	    	  	    if ( value_adc[1] >  value_adc[0]) {//downleft > downright
	    	 	    	  	    	htim1.Instance->CCR1=htim1.Instance->CCR1-10;
	    	 	    	  	     // delay(waittime);
	    	 	    	  	    }
	    	 	    	  	    if ( value_adc[3] <  value_adc[2]) {//topleft < topright
	    	 	    	  	    	htim1.Instance->CCR1=htim1.Instance->CCR1+10;
	    	 	    	  	    //  delay(waittime);
	    	 	    	  	    }
	    	 	    	  	    if ( value_adc[1] <  value_adc[0]) {//downleft < downright
	    	 	    	  	    	htim1.Instance->CCR1=htim1.Instance->CCR1+10;
	    	 	    	  	    //  delay(waittime);
	    	 	    	  	    }
	    	 	    	  	    if (htim1.Instance->CCR1> 1700) {
	    	 	    	  	    	htim1.Instance->CCR1= 1700;
	    	 	    	  	    }
	    	 	    	  	    if (htim1.Instance->CCR1 < 900) {
	    	 	    	  	    	htim1.Instance->CCR1 = 900;
	    	 	    	  	    }
	    	 	    	  //
	    	 	    	  //
	    	 	    	  	    if ( value_adc[3] >  value_adc[1]) {//topleft > downleft
	    	 	    	  	    	htim1.Instance->CCR2 = htim1.Instance->CCR2 + 10;
	    	 	    	  	       // delay(waittime);
	    	 	    	  	      }
	    	 	    	  	      if ( value_adc[2] >  value_adc[0]) {//topright > downright
	    	 	    	  	    	  htim1.Instance->CCR2 = htim1.Instance->CCR2 + 10;
	    	 	    	  	        //delay(waittime);
	    	 	    	  	      }
	    	 	    	  	      if ( value_adc[3] <  value_adc[1]) {//topleft < downleft
	    	 	    	  	    	  htim1.Instance->CCR2 = htim1.Instance->CCR2 - 10;
	    	 	    	  	       // delay(waittime);
	    	 	    	  	      }
	    	 	    	  	      if ( value_adc[2] <  value_adc[0]) {//topright < downright
	    	 	    	  	    	  htim1.Instance->CCR2 = htim1.Instance->CCR2 - 10;
	    	 	    	  	       // delay(waittime);
	    	 	    	  	      }
	    	 	    	  	      if (htim1.Instance->CCR2 > 1400) {
	    	 	    	  	    	  htim1.Instance->CCR2 = 1400;
	    	 	    	  	      }
	    	 	    	  	      if (htim1.Instance->CCR2 < 1000) {
	    	 	    	  	    	  htim1.Instance->CCR2 = 1000;
	    	 	    	  	      }



	    	 	    	  	   start_Time12=HAL_GetTick();}
}
	    	 	    	  }

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
