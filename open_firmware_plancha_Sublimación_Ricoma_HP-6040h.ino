// firmware de control para plancha ricoma hp-6040h
//TODO: sonar alarma al llegar a la temperatura seteada

#include "max6675.h"

//Pines del arduino
#define alarma_timer 3
#define resistencia_plancha 2
//display_1 muestra la temperatura
#define display1_digit1 31
#define display1_digit2 28
#define display1_digit3 29
//display_2 muestra el tiempo de planchado
#define display2_digit1 9
#define display2_digit2 8
#define display2_digit3 30
//Comtrol de led 7 segmentos de 3 digitos a traves de un shif Register
#define clk 7 //rx
#define data 6 //tx en los datos
//pines modulo max6675
#define ktcSO  12
#define ktcCS  10
#define ktcCLK  13
//#define vcc 13 // vcc del max6675
//pines analogos de entrada
#define botonesFrontales A0
#define senal_octoacoplador 5 // señal del switch de contacto

//*digitos de los display*//
#define digito_1 0
#define digito_2 1
#define digito_3 2

const int pantalla_temperatura = 1;
const int pantalla_contador = 2;

const int intervalo_lectura_temp = 500;
unsigned long segundos_contados = 0;
unsigned long tiempo_comienzo_cont = 0;
boolean esta_contando = false;
boolean alarma_activada = false;

//*control plancha*//
//declaramos la estructura ktc, que nos permitira a acceder
//a la tenperatura.

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

unsigned long ultima_lectura = 0;
int temperatura_actual_plancha = 0;
const int segundos_delay_plancha = 5000;
unsigned long ult_delay_plancha = 0;
boolean esta_encendida_plancha = false;
boolean sonar_alarma_en_temp = true;

//*octoacoplador*//


int valor_octoacoplador = 0;

//*Configuraciones*//
int temperatura_plancha = 200;
boolean celssius_temp = true;
int tiempo_contador = 40; //tiempo en segundos que cuenta el reloj de la alarma.


//*menu*//
byte pantalla_actual = 0;
const byte pantalla_principal = 0;
const byte pantalla_menu_temp = 1;
const byte pantalla_menu_c_f = 2;
const byte pantalla_menu_tiempo = 3;


boolean en_menu = false;
int btn_set = 0;
int btn_enter = 0;
int btn_down = 0;
int btn_up = 0;
const int delay_rebote_btn = 50; //tiempo de espera anti rebotes
const int delay_cambio_rapido_btn = 500; //tiempo de espera para cambiar valor rapido
const int valor_valido_btn = 700; //valor valido para el boton

unsigned long tiempo_presion_btn_set = 0;
unsigned long tiempo_presion_btn_enter = 0;
unsigned long tiempo_presion_btn_down = 0;
unsigned long tiempo_presion_btn_up = 0;

boolean estado_btn_set = false;
boolean estado_btn_enter = false;

boolean btn_set_soltado = false;
boolean btn_enter_soltado = false;
boolean btn_down_soltado = false;
boolean btn_up_soltado = false;


boolean mostra_soltado_btn_set = false;
boolean mostra_soltado_btn_enter = false;
boolean mostra_soltado_btn_down = false;
boolean mostra_soltado_btn_up = false;

//*leds*//

boolean led_out = false;
boolean led_at = false;
boolean led_alm = false;
boolean led_end = false;
boolean led_f = false;
boolean led_c = true;


//*PID*//
//P
int y = 0; //y: es igual a temperatura_actual_plancha
int u = 0;
int e = 0;
int ref = 0; //ref: es igual a temperatura_plancha
const int kp = 1;


//*debug*//

int valorboton = 0;
int valorboton_ant = 1;
int promedio = 0;
int maxvalor = 0;

unsigned long ultimo_print = 0;


//bit 1 = pin 3 //E
//bit 2 = pin 4 //D
//bit 3 = pin 5 //Led
//bit 4 = pin 6 //C
//bit 5 = pin 10//G
//bit 6 = pin 11//B
//bit 7 = pin 12//F
//bit 8 = pin 13//A

byte zero  = B00101000;
byte one   = B11101011;
byte two   = B00110010;
byte three = B10100010;
byte four  = B11100001;
byte five  = B10100100;
byte six   = B00100100;
byte seven = B11101010;
byte eight = B00100000;
byte nine  = B11100000;

//byte led_digito  = B11011111;

byte titulo_menu_temp[3] = {
  255,
  B10100100,    // s
  B00101111,    // u
};
byte titulo_menu_tiempo[3] = {
  255,
  B10100100,    // s
  B00110101,    // t
};
byte Digit[10] = {
  B00101000,    // 0
  B11101011,    // 1
  B00110010,    // 2
  B10100010,    // 3
  B11100001,    // 4
  B10100100,    // 5
  B00100100,    // 6
  B11101010,    // 7
  B00100000,    // 8
  B10100000     // 9
};
byte Digit_punto[10] =
{
  B00001000,    // 0
  B11001011,    // 1
  B00010010,    // 2
  B10000010,    // 3
  B11000001,    // 4
  B10000100,    // 5
  B00000100,    // 6
  B11001010,    // 7
  B00000000,    // 8
  B10000000     // 9
};
byte array_display1[3] = {
  display1_digit1,
  display1_digit2,    
  display1_digit3,    
};

byte array_display2[3] = {
  display2_digit1,
  display2_digit2,    
  display2_digit3,    
};

void apagar_displays() {
  digitalWrite(display1_digit1, LOW);
  digitalWrite(display1_digit2, LOW);
  digitalWrite(display1_digit3, LOW);
  digitalWrite(display2_digit1, LOW);
  digitalWrite(display2_digit2, LOW);
  digitalWrite(display2_digit3, LOW);
}

void Display(int pantalla, int pos, int N)
{
  // Apaga todos los digitos
  apagar_displays();

  if (pantalla == pantalla_temperatura) {
    switch (pos) {
      case digito_1://Led Out
        if (led_out) {
          shiftOut(data, clk, MSBFIRST, Digit_punto[N]);
        }
        else {
          shiftOut(data, clk, MSBFIRST, Digit[N]);
        }
        break;
      case digito_2://Led cº
        if (led_c) {
          shiftOut(data, clk, MSBFIRST, Digit_punto[N]);
        }
        else {
          shiftOut(data, clk, MSBFIRST, Digit[N]);
        }
        break;
      case digito_3://Led End
        if (led_end) {
          shiftOut(data, clk, MSBFIRST, Digit_punto[N]);
        }
        else {
          shiftOut(data, clk, MSBFIRST, Digit[N]);
        }
        break;
    }

  } else {
    switch (pos) {
      case digito_1://Led alm
        if (led_alm) {
          shiftOut(data, clk, MSBFIRST, Digit_punto[N]);
        }
        else {
          shiftOut(data, clk, MSBFIRST, Digit[N]);
        }
        break;
      case digito_2://Led fº
        if (led_f) {
          shiftOut(data, clk, MSBFIRST, Digit_punto[N]);
        }
        else {
          shiftOut(data, clk, MSBFIRST, Digit[N]);
        }
        break;
      case digito_3://Led at
        if (led_at) {
          shiftOut(data, clk, MSBFIRST, Digit_punto[N]);
        }
        else {
          shiftOut(data, clk, MSBFIRST, Digit[N]);
        }
        break;
    }

  }



  //      for (int i= 0 ; i<8 ; i++)    // Esto no cambia de la session anterior
  //            digitalWrite(i+2 , Digit[N][i]) ;

  switch (pantalla) {
    case pantalla_temperatura:
      digitalWrite(array_display1[pos], HIGH);
      if (pantalla_actual == pantalla_principal || pantalla_actual == pantalla_menu_tiempo)
        switch (pos) {
          case digito_1://btn_up


            break;
          case digito_2://btn_down
            btn_down = analogRead(botonesFrontales);

            break;
          case digito_3://btn_enter
            btn_enter = analogRead(botonesFrontales);
            /* debug: btn set
              valorboton_ant = valorboton;
              valorboton = btn_enter;
              Serial.print("valor: ");
              Serial.println(valorboton);

              promedio = (valorboton + valorboton_ant) / 2;
              if (promedio > maxvalor) {
              maxvalor = promedio;
              }
            */
            if (btn_enter > valor_valido_btn) { //espresionado el boton

              if (tiempo_presion_btn_enter == 0) {
                tiempo_presion_btn_enter = millis();
              }
              if ((millis() - tiempo_presion_btn_enter) >= delay_rebote_btn) { //esta correctamente presionado
                // Serial.print("estado_btn_enter: ");
                // Serial.println(estado_btn_enter);

                if (btn_enter_soltado && !estado_btn_enter) {
                  mostra_soltado_btn_enter = true;
                  estado_btn_enter = true;
                  btn_enter_soltado = false;
                  if (pantalla_actual == pantalla_principal) {
                    pantalla_actual = pantalla_menu_tiempo;
                    //Serial.println("en el menu tiempo: ");

                  }
                }

              }

            } else {
              if (mostra_soltado_btn_enter) {
                mostra_soltado_btn_enter = false;
                //Serial.println("boton soltado");
              }

              btn_enter_soltado = true;
              tiempo_presion_btn_enter = 0;
            }
            break;
        }
      break;
    case pantalla_contador:
      digitalWrite(array_display2[pos], HIGH);      // Enciende el digito pos
      if (pantalla_actual == pantalla_principal || pantalla_actual == pantalla_menu_temp)
        if (pos == digito_1) {//btn_set

          btn_set = analogRead(botonesFrontales);

          if (btn_set > valor_valido_btn) { //espresionado el boton

            if (tiempo_presion_btn_set == 0) {
              tiempo_presion_btn_set = millis();
            }
            if ((millis() - tiempo_presion_btn_set) >= delay_rebote_btn) { //esta correctamente presionado
              if (alarma_activada) { //silencia la alarma con el boton set
                silenciar_alarma();

              }

              // Serial.print("estado_btn_set: ");
              // Serial.println(estado_btn_set);

              if (btn_set_soltado && !estado_btn_set) {
                mostra_soltado_btn_set = true;
                estado_btn_set = true;
                btn_set_soltado = false;

                pantalla_actual = pantalla_menu_temp;
                //Serial.println("en el menu: ");


              } else {
                if (btn_set_soltado && estado_btn_set)
                {
                  mostra_soltado_btn_set = true;
                  estado_btn_set = false;
                  btn_set_soltado = false;
                  //Serial.println("en la pantalla principal");


                  pantalla_actual = pantalla_principal;
                }
              }




            }

          } else {
            if (mostra_soltado_btn_set) {
              mostra_soltado_btn_set = false;
              //Serial.println("boton soltado");
            }

            btn_set_soltado = true;
            tiempo_presion_btn_set = 0;
          }

        }

      break;
  }

  //      digitalWrite(pos + display1_digit1, HIGH);      // Enciende el digito pos
}
void CalculaDigitos( int Num, int pantalla)
{
  //Serial.print("Numero ");
  //Serial.print(Num);
  //Serial.println(": ");
  int Digit3 = Num % 10 ;
  int Digit2 = (Num % 100) / 10 ;
  int Digit1 = (Num % 1000) / 100 ;

  switch (pantalla) {
    case pantalla_temperatura:
      Display(pantalla_temperatura, digito_1, Digit1);
      //delay(300);
      Display(pantalla_temperatura, digito_2, Digit2);
      //delay(300);
      Display(pantalla_temperatura, digito_3, Digit3);
      //delay(300);
      break;
    case pantalla_contador:
      Display(pantalla_contador, digito_1, Digit1);
      //(300);
      Display(pantalla_contador, digito_2, Digit2);
      //(300);
      Display(pantalla_contador, digito_3, Digit3);
      //(300);
      break;
  }


}



void contar_segundos() {
  int tiempo_restante = 0;
  valor_octoacoplador = digitalRead(senal_octoacoplador);
  led_alm = true;
  if (valor_octoacoplador == LOW) {
    if (!esta_contando) {
      esta_contando = true;
      tiempo_comienzo_cont = millis();

    }
    if (!alarma_activada) {
      segundos_contados = (millis() - tiempo_comienzo_cont) / 1000;
      tiempo_restante = tiempo_contador - segundos_contados;
      CalculaDigitos(tiempo_restante, pantalla_contador);
      if (tiempo_restante <= 0) {
        alarma_activada = true;

      }

    } else {
      CalculaDigitos(0, pantalla_contador); //muestra cero por que se acabo el tiempo
      sonar_alarma();//suena la alarma

    }


  } else {



    silenciar_alarma();

    CalculaDigitos(tiempo_contador, pantalla_contador);
  }


}
void sonar_alarma() {
  led_end = true;
  digitalWrite(alarma_timer, HIGH);
}

void silenciar_alarma() {
  alarma_activada = false;
  esta_contando = false;
  tiempo_comienzo_cont = 0;
  segundos_contados = 0;
  led_alm = false;
  led_end = false;
  digitalWrite(alarma_timer, LOW);
}

void mostrar_temperatura() {


  CalculaDigitos(temperatura_actual_plancha, pantalla_temperatura);
}

void encender_plancha() {
  if (!esta_encendida_plancha) {
    esta_encendida_plancha = true;
    led_out = true;
    digitalWrite(resistencia_plancha, HIGH);
  }

}
void apagar_plancha() {
  if (esta_encendida_plancha) {
    esta_encendida_plancha = false;
    led_out = false;
    ult_delay_plancha = millis();
    digitalWrite(resistencia_plancha, LOW);
  }
}

void comprobar_temp() {
  unsigned long tiempo_lectura = millis();
  if ((tiempo_lectura - ultima_lectura) > intervalo_lectura_temp) {
    if (celssius_temp) {
      temperatura_actual_plancha = ktc.readCelsius();
    } else {
      temperatura_actual_plancha = ktc.readFahrenheit();
    }
    ultima_lectura = millis();

    /*
      unsigned long tiempo_transcurrido_delay_plancha = millis() - ult_delay_plancha;

      if (temperatura_actual_plancha < temperatura_plancha && tiempo_transcurrido_delay_plancha < segundos_delay_plancha) {
      encender_plancha();

      } else {
      apagar_plancha();

      }
    */
    y = temperatura_actual_plancha;
    ref = temperatura_plancha;
    e = ref - y;
    u = kp * e;
    if (u > 0) {
      encender_plancha();
    } else {
      if (sonar_alarma_en_temp) {
        sonar_alarma_en_temp = false;

        sonar_alarma();

      }
      apagar_plancha();
    }
  }
}


void mostrar_titulo(byte titulo[]) {
  for (int i = 0; i < 3; i++) {
    apagar_displays();
    shiftOut(data, clk, MSBFIRST, titulo[i]);
    digitalWrite(i + display1_digit1, HIGH);
    switch (i) {
      case 0:
        btn_up = analogRead(botonesFrontales);

        if (btn_up > valor_valido_btn) { //espresionado el boton
          //Serial.println("presionado el btn_up");

          if (tiempo_presion_btn_up == 0) {
            tiempo_presion_btn_up = millis();
          }
          if ((millis() - tiempo_presion_btn_up) >= delay_rebote_btn) { //esta correctamente presionado

            if (btn_up_soltado) {
              mostra_soltado_btn_up = true;
              btn_up_soltado = false;
              switch (pantalla_actual) {
                case pantalla_menu_temp:
                  temperatura_plancha++;
                  sonar_alarma_en_temp = true;

                  break;
                case pantalla_menu_c_f:
                  celssius_temp = !celssius_temp;

                  break;
                case pantalla_menu_tiempo:
                  tiempo_contador++;

                  break;
              }

            }

          }

        } else {
          if (mostra_soltado_btn_up) {
            mostra_soltado_btn_up = false;
            //Serial.println("boton soltado");
          }

          btn_up_soltado = true;
          tiempo_presion_btn_up = 0;
        }

        //*/
        break;
      case 1:
        btn_down = analogRead(botonesFrontales);

        if (btn_down > valor_valido_btn) { //espresionado el boton
          //Serial.println("presionado el btn_down");

          if (tiempo_presion_btn_down == 0) {
            tiempo_presion_btn_down = millis();
          }
          if ((millis() - tiempo_presion_btn_down) >= delay_rebote_btn) { //esta correctamente presionado

            if (btn_down_soltado) {
              mostra_soltado_btn_down = true;
              btn_down_soltado = false;
              switch (pantalla_actual) {
                case pantalla_menu_temp:
                  temperatura_plancha--;
                  sonar_alarma_en_temp = true;

                  break;
                case pantalla_menu_c_f:
                  celssius_temp = !celssius_temp;

                  break;
                case pantalla_menu_tiempo:
                  tiempo_contador--;

                  break;
              }

            }

          }

        } else {
          if (mostra_soltado_btn_down) {
            mostra_soltado_btn_down = false;
            //Serial.println("boton soltado");
          }

          btn_down_soltado = true;
          tiempo_presion_btn_down = 0;
        }

        //*/
        break;
      case 2:

        btn_enter = analogRead(botonesFrontales);
        /* debug: btn set
          valorboton_ant = valorboton;
          valorboton = btn_enter;
          Serial.print("valor: ");
          Serial.println(valorboton);

          promedio = (valorboton + valorboton_ant) / 2;
          if (promedio > maxvalor) {
          maxvalor = promedio;
          }
        */
        if (btn_enter > valor_valido_btn) { //espresionado el boton

          if (tiempo_presion_btn_enter == 0) {
            tiempo_presion_btn_enter = millis();
          }
          if ((millis() - tiempo_presion_btn_enter) >= delay_rebote_btn) { //esta correctamente presionado
            // Serial.print("estado_btn_enter: ");
            // Serial.println(estado_btn_enter);

            if (btn_enter_soltado && estado_btn_enter)


            {
              mostra_soltado_btn_enter = true;
              estado_btn_enter = false;
              btn_enter_soltado = false;
              //Serial.println("en la pantalla principal");


              pantalla_actual = pantalla_principal;
            }

          }

        } else {
          if (mostra_soltado_btn_enter) {
            mostra_soltado_btn_enter = false;
            //Serial.println("boton soltado");
          }

          btn_enter_soltado = true;
          tiempo_presion_btn_enter = 0;
        }

        break;
    }
  }
}
void menu_set_temp() {
  mostrar_titulo(titulo_menu_temp);
  CalculaDigitos(temperatura_plancha, pantalla_contador);
}
void menu_set_tiempo() {
  mostrar_titulo(titulo_menu_tiempo);
  CalculaDigitos(tiempo_contador, pantalla_contador);
}




void setup() {
  pinMode(alarma_timer , OUTPUT);
  pinMode(resistencia_plancha , OUTPUT);
  pinMode(senal_octoacoplador , INPUT);
  //  digitalWrite(resistencia_plancha,LOW);
  //  digitalWrite(alarma_timer,LOW);
  pinMode(clk, OUTPUT); // make the clock pin an output
  pinMode(data , OUTPUT); // make the data pin an output
  pinMode(display1_digit1 , OUTPUT); // make the data pin an output
  //  digitalWrite(display1_digit1,HIGH);
  pinMode(display1_digit2 , OUTPUT); // make the data pin an output
  //  digitalWrite(display1_digit2,HIGH);
  pinMode(display1_digit3 , OUTPUT); // make the data pin an output
  //  digitalWrite(display1_digit3,HIGH);
  pinMode(display2_digit1 , OUTPUT); // make the data pin an output
  //  digitalWrite(display2_digit1,HIGH);
  pinMode(display2_digit2 , OUTPUT); // make the data pin an output
  //  digitalWrite(display2_digit2,HIGH);
  pinMode(display2_digit3 , OUTPUT); // make the data pin an output
  //  digitalWrite(display2_digit3,HIGH);

  //seteamos los pines del sensor 6675
  //digitalWrite(vcc, HIGH); // le damos energia al sensor
  //Serial.begin(9600);//debug: esa linea mantiene activado los pines digitales 0 y 1(Tx,Rx)
  // give the MAX a little time to settle
  delay(500);

}

void loop() {
  unsigned long tiempo1, tiempo2, tiempo3, tiempo4;

  ///*
  //tiempo1=millis();
  switch (pantalla_actual) { //cambia el estado de los display en la IU
    case pantalla_principal:
      comprobar_temp();
      //    tiempo2=millis();
      contar_segundos();
      //   tiempo3=millis();
      mostrar_temperatura();
      // tiempo4=millis();
      break;
    case pantalla_menu_temp:
      menu_set_temp();
      break;
    case pantalla_menu_c_f:
      break;
    case pantalla_menu_tiempo:
      menu_set_tiempo();
      break;
  }
  /*
    if((millis()-ultimo_print)>1000){//imprime la diferencia del tiempo
    Serial.print("intervalo inicio_comprobar_temp:");
    Serial.print(tiempo1-tiempo2);
    Serial.print(", intervalo comprobar_temp: ");
    Serial.print("");
    Serial.print("");
    Serial.println("");
    }
  */
  // */
  //digitalWrite(display1_digit2,HIGH);
  //    digitalWrite(display1_digit3,HIGH);
  //
  //   shiftOut(data, clk, MSBFIRST, zero);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, one);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, two);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, three);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, four);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, five);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, six);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, seven);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, eight);
  //    (500);
  //    shiftOut(data, clk, MSBFIRST, nine);
  //    (500);
  //CalculaDigitos( millis() / 1000);
  //Serial.print("tiempo = ");
  //Serial.println((millis() / 1000));
  /*shiftOut(data, clk, MSBFIRST, zero);
    digitalWrite(alarma_timer,HIGH);
    (5000);
    digitalWrite(alarma_timer,LOW);
    shiftOut(data, clk, MSBFIRST, one);
    digitalWrite(resistencia_plancha,HIGH);
    (5000);
    digitalWrite(resistencia_plancha,LOW);
  */


  /* debug: botones





    CalculaDigitos(maxvalor, pantalla_temperatura);
    CalculaDigitos(promedio, pantalla_contador);
    /*/
}
