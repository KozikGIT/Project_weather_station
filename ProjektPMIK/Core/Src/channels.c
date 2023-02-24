
#include "channels.h"

int licznik_zmiany_daty=0;
uint start_Time10;
uint rok1=21;
	uint miesiac1=6;
	uint dzien1=22;
	uint godzina1=4;
	uint minuta1=12;
	uint sekunda1=12;
extern int licznik_kanalu;
extern int czy_wyswietlono;
extern int ustaw_panel;
extern pobierz_dane;
/**
  * @brief
  * Funnkcja w której dzieją się poszczególne rzeczy po wciśnieciu przyisków na pilocie
  * @retval None
  */
void channel_of_display()
{
	  int value23 = ir_read();



		    if (value23 != -1) {
		      printf("code = %d\n", value23);
		      czy_wyswietlono=0;


if(value23==0x4A) //przycisk 9
{ustaw_panel=(ustaw_panel+1) %2;
	}

if(value23==0x52) //przycisk 8
{
	pobierz_dane=1;
	}

if(value23==0x42) //przycisk 7
{
	pobierz_dane=2;
	}


	if(licznik_kanalu>-2)
	{
		if(value23==7)
				    {
				    	printf("lewo \n");
				    	licznik_kanalu--;
				    }


	}
	else
	{
		licznik_kanalu=-2;
	}

	if(licznik_kanalu<2)
	{
		if(value23==9)
				   	    {
				   	    	printf("prawo \n");
				   	    	licznik_kanalu++;
				   	    }

	}
	else
		{
			licznik_kanalu=2;

		}





	if(value23==13) //Przycisk C
	{
		if(licznik_zmiany_daty>=0 && licznik_zmiany_daty<=6)
		licznik_zmiany_daty++;

	}
	if(value23==21) //Przycisk Play zatwierdzenie daty i godziny
		{
		RTC_TimeTypeDef new_time = {0};
	  	RTC_DateTypeDef new_date={0};
	  	set_new_date_and_time(new_date,new_time,rok1,miesiac1,dzien1,godzina1,minuta1,sekunda1);
			licznik_zmiany_daty=0;

		}


	if(licznik_zmiany_daty==1)
	{


						if(value23==64) //+
						{
							rok1++;
						}

							if(value23==25) //-
							{	rok1--;
							}

		printf("rok: %d \n",rok1);
	}
	if(licznik_zmiany_daty==2)
		{


							if(value23==64) //+
							{

								miesiac1++;
								if(miesiac1>12)
								{
									miesiac1=12;
								}

							}

								if(value23==25) //-
								{	miesiac1--;
								if(miesiac1<1)
																{
																	miesiac1=1;
																}
								}

			printf("miesiac: %d \n",miesiac1);
		}

	if(licznik_zmiany_daty==3)
		{


							if(value23==64) //+     //do poprawy maksymalna ilosc dni
							{
								dzien1++;
								if(dzien1>31)
								{
									dzien1=31;
								}
							}

								if(value23==25) //-
								{	dzien1--;
								if(dzien1<1)
																{
																	dzien1=1;
																}
								}

			printf("dzien: %d \n",dzien1);
		}

	if(licznik_zmiany_daty==4)
		{


							if(value23==64) //+
							{
								godzina1++;
								if(godzina1>23)
																{
																	godzina1=23;
																}
							}

								if(value23==25) //-
								{	godzina1--;
								if(godzina1<0)
									{
										godzina1=0;
									}
								}

			printf("godzina: %d \n",godzina1);
		}

	if(licznik_zmiany_daty==5)
		{


							if(value23==64) //+
							{
								minuta1++;
								if(minuta1>59)
																	{
																		minuta1=59;
																	}
							}

								if(value23==25) //-
								{	minuta1--;
								if(minuta1<0)
																									{
																										minuta1=0;
																									}
								}

			printf("minuta: %d \n",minuta1);
		}

	if(licznik_zmiany_daty==6)
		{


							if(value23==64) //+
							{
								sekunda1++;
								if(sekunda1>59)
																									{
																										sekunda1=59;
																									}
							}

								if(value23==25) //-
								{	sekunda1--;
								if(sekunda1<0)
																									{
																										sekunda1=0;
																									}
								}

			printf("sekunda: %d \n",sekunda1);
		}








	printf("licznik_kanalu wynosi: %d \n",licznik_kanalu);






		    }
	}






