
#include "usart2_comunication.h"

	uint rok;
	uint miesiac;
	uint dzien;
	uint godzina;
	uint minuta;
	uint sekunda;
	int wybor_daty;
	int wybor_miesiaca;
	int wybor_dnia;
	int wybor_sekundy;
	int wybor_minuty;
	int wybor_godziny;

extern int onlywrite;

//nadpisanie io_putchar tak aby mozna bylo za pomoca USART uzywac funkcji "printf"
int __io_putchar(int ch)
{
    if (ch == '\n') {
        uint8_t ch2 = '\r';
        HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
    }

    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return 1;
}


//tworzenie calego wyrazu z liter wpisanych na konsoli i zaakceptowanych enterem i porownywanie wyrazu z czyms

#define LINE_MAX_LENGTH	80

static char line_buffer[LINE_MAX_LENGTH + 1];
static uint32_t line_length;

void line_append(uint8_t value)
{
	if (value == '\r' || value == '\n') {
		// odebraliśmy znak końca linii
		if (line_length > 0) {
			// jeśli bufor nie jest pusty to dodajemy 0 na końcu linii
			line_buffer[line_length] = '\0';
			// przetwarzamy dane


					if(strcmp(line_buffer,"just write")==0)
	{
			onlywrite=1;
		printf("teraz bez odczytu \n");
	}

					if(strcmp(line_buffer,"also reading")==0)
		{
				onlywrite=0;
			printf("teraz odczyt i pisanie \n");
		}



					if(strcmp(line_buffer,"rok")==0)
							{
									wybor_daty=1;
							}
					if(strcmp(line_buffer,"nierok")==0)
							{
									wybor_daty=0;
							}
					if(strcmp(line_buffer,"miesiac")==0)
							{
									wybor_miesiaca=1;
							}
					if(strcmp(line_buffer,"niemiesiac")==0)
							{
									wybor_miesiaca=0;
							}

					if(strcmp(line_buffer,"dzien")==0)
							{
									wybor_dnia=1;
							}
					if(strcmp(line_buffer,"niedzien")==0)
							{
									wybor_dnia=0;
							}

					if(strcmp(line_buffer,"godzina")==0)
							{
									wybor_godziny=1;
							}
					if(strcmp(line_buffer,"niegodzina")==0)
							{
									wybor_godziny=0;
							}
					if(strcmp(line_buffer,"minuta")==0)
							{
									wybor_minuty=1;
							}
					if(strcmp(line_buffer,"nieminuta")==0)
							{
									wybor_minuty=0;
							}
					if(strcmp(line_buffer,"sekunda")==0)
							{
									wybor_sekundy=1;
							}
					if(strcmp(line_buffer,"niesekunda")==0)
							{
									wybor_sekundy=0;
							}






		if(wybor_daty==1){


			rok=atoi(line_buffer);
			if(rok>=0 && rok<=99){

			}   //dziala
			else{rok=22;}
		}

		if(wybor_miesiaca==1)
		{

			miesiac=atoi(line_buffer);
			if(miesiac>=1 && miesiac<=12){

			}   //dziala
			else{miesiac=6;}

			}
		if(wybor_dnia==1)
		{
			dzien=atoi(line_buffer);
			if(dzien>=1 && dzien<=31){

			}   //dziala
			else{dzien=15;}
			}


		if(wybor_godziny==1)
				{
					godzina=atoi(line_buffer);
					if(godzina>=0 && godzina<=24){

					}   //dziala
					else{godzina=15;}
					}

		if(wybor_minuty==1)
						{
							minuta=atoi(line_buffer);
							if(minuta>=0 && godzina<=59){

							}   //dziala
							else{minuta=15;}
							}

		if(wybor_sekundy==1)
								{
									sekunda=atoi(line_buffer);
									if(sekunda>=0 && sekunda<=59){

									}   //dziala
									else{sekunda=15;}
									}








		printf("aktualna data:rok: %d, miesiac: %d, dzien: %d, godzina: %d, minuta: %d, sekunda: %d,\n", rok,miesiac,dzien,godzina,minuta,sekunda);




			printf("Otrzymano: %s\n", line_buffer);


			if(strcmp(line_buffer,"set_date")==0)
												{
			 	RTC_TimeTypeDef new_time = {0};
						  	RTC_DateTypeDef new_date={0};
			  	 set_new_date_and_time(new_date,new_time,rok,miesiac,dzien,godzina,minuta,sekunda);





												}




		// zaczynamy zbieranie danych od nowa
			line_length = 0;
		}
	}
	else {
		if (line_length >= LINE_MAX_LENGTH) {
			// za dużo danych, usuwamy wszystko co odebraliśmy dotychczas
			line_length = 0;
		}
		// dopisujemy wartość do bufora
		line_buffer[line_length++] = value;


	}

}








//tej funkcji uzywamy w miejscu, gdzie chcemy wyswietlic to co wpisalismy w konsoli
void  print_your_console_write()
{

    uint8_t value;
	  if (HAL_UART_Receive(&huart2, &value, 1, 0) == HAL_OK)
		  line_append(value);


}

