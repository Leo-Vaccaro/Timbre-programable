#include <EEPROM.h>
#include <LiquidMenu.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#define CLK   5
#define DT    4
#define SW    3
#define ENTRADA 2
#define Alarmin 13

LiquidCrystal_I2C lcd(0x27,16,2);
RTC_DS3231 rtc;

void alarma();
void hora();
void minutos();
int alarms[10] [5]={{0,0,0,13,54},{0,0,0,23,9},{0,0,0,2,18},{0,0,0,6,47},{0,0,0,20,24},{0,0,0,16,30},{0,0,0,13,54},{10,0,0,7,0},{0,0,0,22,45},{0,0,0,10,15}};
const int alarmapin=1;
volatile int stalarm=0;
int contador=0,flag_while=0; 
int flag_sw=0;
int sw_menu=0;
int num_alarma=0;
int last_flag_sw=0;
int clk_state=0, clk_last_state=0;
int b=1, y=0,x=0, c=0;
int actDesAlarma[10]={1,1,1,1,1,1,1,1,1,1};
int option=0;//verifica si este en edicion, agregar o eliminar.
// agregar=1    editar=2   eliminar=3

void onAlarm(){
  stalarm=1;
}


LiquidLine line_1(1,0,"Menu de Alarmas");
LiquidLine line_2(1,0,"Editar Alarma");
LiquidLine line_3(1,0,"Act/Des Alarm.");
LiquidLine line_4(1,0,"Volver");
LiquidScreen screen_1(line_1, line_2, line_3, line_4);

LiquidLine line2_1(1,0,"Horario");
LiquidLine line2_2(1,1,"Volver");
LiquidScreen screen_2(line2_1, line2_2);

LiquidLine line3_1(1,0,"Selec. Alarma");
LiquidLine line3_2(1,1,"Volver");
LiquidScreen screen_3(line3_1, line3_2);

LiquidMenu menu(lcd,screen_1,screen_2,screen_3);


void setup() {
  //setuo rct y lcd
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  if(!rtc.begin()){
    Serial.print("CAPO NO se encontro el RTC, denada");
    Serial.flush();
    abort();
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.disable32K();//desabilita las señales del pin de 32k que esta al pedo
  pinMode(ENTRADA,INPUT_PULLUP);//configuracion del modo de trabajo del pin a usar como interruptor 
  pinMode(Alarmin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENTRADA),onAlarm,FALLING);//creacion y configuracion de el interruptor
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.clearAlarm(1);
  rtc.setAlarm1(DateTime(2023,6,27,19,54,0),DS3231_A1_Hour);
  //setup menu
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  menu.init();
  
  line_1.set_focusPosition(Position::LEFT);
  line_2.set_focusPosition(Position::LEFT);
  line_3.set_focusPosition(Position::LEFT);
  line_4.set_focusPosition(Position::LEFT);

  line_1.attach_function(1, fn_menu);
  line_2.attach_function(1, fn_editar);
  line_3.attach_function(1, fn_des_act);
  line_4.attach_function(1, fn_volver_menu); 

  menu.add_screen(screen_1);

  line2_1.set_focusPosition(Position::LEFT);
  line2_2.set_focusPosition(Position::LEFT);

  line2_1.attach_function(1, fn_horario);
  line2_2.attach_function(1, fn_volver);

  menu.add_screen(screen_2);

  line3_1.set_focusPosition(Position::LEFT);
  line3_2.set_focusPosition(Position::LEFT);

  line3_1.attach_function(1, fn_selec_alarm);
  line3_2.attach_function(1, fn_volver);

  menu.add_screen(screen_3);
  
  screen_1.set_displayLineCount(2);
  screen_2.set_displayLineCount(2);
  screen_3.set_displayLineCount(2);
  
  menu.set_focusedLine(0);
  menu.update();
  /*for (int i=0; i<10;i++){
    Serial.println(alarms[i] [3]);
    Serial.println(alarms[i] [4]);
    Serial.println("asdwdawd");
    EEPROM.write(b,alarms[i] [3]);
    EEPROM.write(b+1,alarms[i] [4]);
    b=b+2; 
  }*/
}

void loop() {
  while(flag_sw==0){
      if(digitalRead(SW)==LOW){
        delay(50);
        if(digitalRead(SW)==LOW){
          lcd.clear();
          flag_sw=1;
        }
      }
      sig_alarma();
      DateTime now= rtc.now();
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print(now.day());
      lcd.print("/");
      lcd.print(now.month());
      lcd.print("/");
      lcd.print(now.year());
      lcd.print(" ");
      lcd.setCursor(10, 0); 
      lcd.print("S.A:");
      lcd.print(c);
      //lcd.print(el numero de alarma sig)
      lcd.setCursor(0, 1); //cambiamos el renglon para que no se sobrepongan las cosas en el lcd
      lcd.print(now.hour());
      lcd.print(":");
      lcd.print(now.minute());
      lcd.print(":");
      lcd.print(now.second());
      delay(200);
  }
  selectOption();

  clk_state = digitalRead(CLK); 
    if (clk_state != clk_last_state){     
      if (digitalRead(DT) != clk_state) { 
        menu.switch_focus(true);
      } else {
        menu.switch_focus(false);
      }
      menu.update();
      clk_last_state = clk_state;
  }
}

//Funciones:::::

void fn_des_act(){
  sw_menu=3;
  fn_selec_alarm();
}

void sig_alarma(){
  int messi=0;
  DateTime now= rtc.now();
  //0=off
  //1=on
  switch(c){
    case 0:
      for(int i=0;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
      }
    break;
    case 1:
      for(int i=1;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=9){
        if(actDesAlarma[0]==1){
          b=0;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
      }
    break;
    case 2:
      for(int i=2;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=8){
        for(int i=0;i<2;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 3:
      for(int i=4;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=7){
        for(int i=0;i<3;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 4:
      for(int i=4;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=6){
        for(int i=0;i<4;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 5:
      for(int i=5;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=5){
        for(int i=0;i<5;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 6:
      for(int i=6;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=4){
        for(int i=0;i<6;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 7:
      for(int i=7;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=3){
        for(int i=0;i<7;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 8:
      for(int i=8;i<10;i++){
        if(actDesAlarma[i]==1){
          b=i;//se iguala el contador el for  un variable
          swicht_case();
          break;
        }
        else{
          messi++;
        }
      }
      if(messi=2){
        for(int i=0;i<8;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
    case 9:
      if(actDesAlarma[9]==1){   
          x=19;
          y=20;
      }
      else{
        for(int i=0;i<9;i++){
          if(actDesAlarma[i]==1){
            b=i;//se iguala el contador el for  un variable
            swicht_case();
            break;
          }
        }
      }
    break;
  }
  Serial.println("ELGARCHADO");
  Serial.println(actDesAlarma[0]);
  Serial.println("MEssi");
  Serial.println(EEPROM.read(x));
  Serial.println(EEPROM.read(y));
  rtc.setAlarm1(DateTime(now.year(),now.month(),now.day(),EEPROM.read(x),EEPROM.read(y),0),DS3231_A1_Hour);
  alarma();
}

void selectOption(){
  if(digitalRead(SW) == LOW){
      lcd.clear();
      menu.call_function(1);
      delay(500);
  }
}

void swicht_case(){
  switch(b){
    case 0:
      x=1;
      y=2;
    break;
    case 1:
      x=3;
      y=4;
    break;
    case 2:
      x=5;
      y=6;
    break;
    case 3:
      x=7;
      y=8;
    break;
    case 4:
      x=9;
      y=10;
    break;
    case 5:
      x=11;
      y=12;
    break;
    case 6:
      x=13;
      y=14;
    break;
    case 7:
      x=15;
      y=16;
    break;
    case 8:
      x=17;
      y=18;
    break;
    case 9:
      x=19;
      y=20;
    break;
  }
}

void fn_menu(){
  sw_menu=2;
  menu.change_screen(3);
  menu.set_focusedLine(0);
}

void fn_editar(){
  sw_menu=1;
  menu.change_screen(3);
  menu.set_focusedLine(0);
}



void hora(){
  contador=0;
  while(flag_while==0){
    lcd.setCursor(0,0);
    clk_state = digitalRead(CLK); 
    if (clk_state != clk_last_state){     
      if (digitalRead(DT) != clk_state) { 
        contador++;
        if(contador>24){
          lcd.clear();
          contador=0;
        }
      } 
      else {
        contador--;
        if(contador<0){
          contador=24;
        }
      }
        clk_last_state = clk_state;
    }
    if(contador<10){
       lcd.setCursor(0,0);
       lcd.print("0");
       lcd.setCursor(1,0);
    }
    lcd.print(contador);
    lcd.print(":");
    lcd.setCursor(3,0);
    lcd.print("0");
    lcd.print("0");
    lcd.setCursor(5,0);
    lcd.print(":");
    lcd.print("0");
    lcd.print("0");
    if(digitalRead(SW)==LOW){
      delay(100);
      if(digitalRead(SW)==LOW){
        flag_while=1;
    }
   }
  }
  EEPROM.write(x,contador);
  Serial.println("AAAAAAAAAAAA");
 // Serial.println(alarms[1] [3]);
  Serial.println(contador);
}

void minutos(){
  contador=0;
  while(flag_while==0){
    lcd.setCursor(3,0);
    clk_state = digitalRead(CLK); 
    if (clk_state != clk_last_state){     
      if (digitalRead(DT) != clk_state) { 
        contador++;
        if(contador>60){
          contador=0;
        }
      } 
      else {
        contador--;
        if(contador<0){
          contador=60;
        }
      }
        clk_last_state = clk_state;
    }
    if(contador<10){
       lcd.setCursor(3,0);
       lcd.print("0");
       lcd.setCursor(4,0);
    }
    lcd.print(contador);
    lcd.print(":");
    lcd.setCursor(5,0);
    lcd.print(":");
    lcd.print("0");
    lcd.print("0");
    if(digitalRead(SW)==LOW){
      delay(100);
      if(digitalRead(SW)==LOW){
        flag_while=1;
    }
   }
  }
  EEPROM.write(y,contador);
  Serial.println("contador mins:");
  Serial.println(contador);
  //Serial.println(alarms[1] [4]);
}

void fn_volver(){
  menu.change_screen(1);
  menu.set_focusedLine(0);
}
void fn_volver_menu(){
  flag_sw=0;
}
void fn_horario(){
  DateTime now= rtc.now();
  flag_while=0;
  hora();
  flag_while=0;
  minutos();
  //rtc.setAlarm1(DateTime(2023,now.month(),now.day(),EEPROM.read(x),EEPROM.read(y),0),DS3231_A1_Hour);
  menu.change_screen(3);
  menu.set_focusedLine(0);
}

void fn_selec_alarm(){
  contador=0;
  flag_while=0;
  while(flag_while==0){
    lcd.setCursor(0,0);
    lcd.print("Selec. Alarma");
    lcd.setCursor(7,1);
    clk_state = digitalRead(CLK); 
    if (clk_state != clk_last_state){     
      if (digitalRead(DT) != clk_state) { 
        contador++;
        if(contador>10){
          lcd.clear();
          contador=0;
        }
      } 
      else {
        contador--;
        if(contador<0){
          contador=9;
        }
      }
      clk_last_state = clk_state;
    }
    if(contador<10){
       lcd.setCursor(7,1);
       lcd.print("0");
       lcd.setCursor(8,1);
    }
    lcd.print(contador);
    if(digitalRead(SW)==LOW){
      delay(150);
      if(digitalRead(SW)==LOW){
        flag_while=1;
    }
   }
  }
  num_alarma=contador;
  delay(75);
  if(sw_menu==1){
    switch(num_alarma){
      case 0:
        x=1;
        y=2;
      break;
      case 1:
        x=3;
        y=4;
      break;
      case 2:
        x=5;
        y=6;
      break;
      case 3:
        x=7;
        y=8;
      break;
      case 4:
        x=9;
        y=10;
      break;
      case 5:
        x=11;
        y=12;
      break;
      case 6:
        x=13;
        y=14;
      break;
      case 7:
        x=15;
        y=16;
      break;
      case 8:
        x=17;
        y=18;
      break;
      case 9:
        x=19;
        y=20;
      break;
    }
    lcd.clear();
    fn_horario();
  }
  if(sw_menu==2){
    b=contador;
    fn_menu_a();
  }
  b=contador;
  c=contador;
  if(sw_menu==3){
    switch(c){
      case 0:
        if(actDesAlarma[0]==1){
          actDesAlarma[0]=0; 
          Serial.print("ADASDASDSADSA");
          Serial.print("ADASDASDSADSA");
          Serial.print("ADASDASDSADSA");
          Serial.print("ADASDASDSADSA");
          Serial.print("ALARMA 0 ESACTIVDA");
        }
        else{
          actDesAlarma[0]=1;
        }
      break;
      case 1:
        if(actDesAlarma[1]==1){
          actDesAlarma[1]=0; 
        }
        else{
          actDesAlarma[1]=1;
        }
      break;
      case 2:
        if(actDesAlarma[2]==1){
          actDesAlarma[2]=0; 
        }
        else{
          actDesAlarma[2]=1;
        }
      break;
      case 3:
        if(actDesAlarma[3]==1){
          actDesAlarma[3]=0; 
        }
        else{
          actDesAlarma[3]=1;
        }
      break;
      case 4:
        if(actDesAlarma[4]==1){
          actDesAlarma[4]=0; 
        }
        else{
          actDesAlarma[4]=1;
        }
      break;
      case 5:
        if(actDesAlarma[5]==1){
          actDesAlarma[5]=0; 
        }
        else{
          actDesAlarma[5]=1;
        }
      break;
      case 6:
        if(actDesAlarma[6]==1){
          actDesAlarma[6]=0; 
        }
        else{
          actDesAlarma[6]=1;
        }
      break;
      case 7:
        if(actDesAlarma[7]==1){
          actDesAlarma[7]=0; 
        }
        else{
          actDesAlarma[7]=1;
        }
      break;
      case 8:
        if(actDesAlarma[8]==1){
          actDesAlarma[8]=0; 
        }
        else{
          actDesAlarma[8]=1;
        }
      break;
      case 9:
        if(actDesAlarma[9]==1){
          actDesAlarma[9]=0; 
        }
        else{
          actDesAlarma[9]=1;
        }
      break;
    }
  }
  delay(50);
  if(digitalRead(SW)==LOW){
    delay(150);
    if(digitalRead(SW)==LOW){
      flag_while=0;
      lcd.clear();
    }
  }
}


void fn_menu_a(){
  while(true){
    DateTime now= rtc.now();
    lcd.setCursor(0,0);
    lcd.print("Num de Alarma:");
    lcd.setCursor(14,0);
    lcd.print(num_alarma);
    lcd.setCursor(0,1);
    switch(b){
      case 0:
        lcd.print(EEPROM.read(1));
        lcd.print(":");
        lcd.print(EEPROM.read(2));
      break;
      case 1:
        lcd.print(EEPROM.read(3));
        lcd.print(":");
        lcd.print(EEPROM.read(4));
      break;
      case 2:
        lcd.print(EEPROM.read(5));
        lcd.print(":");
        lcd.print(EEPROM.read(6));
      break;
      case 3:
        lcd.print(EEPROM.read(7));
        lcd.print(":");
        lcd.print(EEPROM.read(8));
      break;
      case 4:
        lcd.print(EEPROM.read(9));
        lcd.print(":");
        lcd.print(EEPROM.read(10));
      break;
      case 5:
        lcd.print(EEPROM.read(11));
        lcd.print(":");
        lcd.print(EEPROM.read(12));
      break;
      case 6:
        lcd.print(EEPROM.read(13));
        lcd.print(":");
        lcd.print(EEPROM.read(14));
      break;
      case 7:
        lcd.print(EEPROM.read(15));
        lcd.print(":");
        lcd.print(EEPROM.read(16));
      break;
      case 8:
        lcd.print(EEPROM.read(17));
        lcd.print(":");
        lcd.print(EEPROM.read(18));
      break;
      case 9:
        lcd.print(EEPROM.read(19));
        lcd.print(":");
        lcd.print(EEPROM.read(20));
      break;
    }
    lcd.print(":");
    lcd.print("00");
    lcd.print("   ");
    if(digitalRead(SW)==LOW){
      delay(150);
      if(digitalRead(SW)==LOW){
        lcd.clear();
        break;
      }
    }
  }
}

void alarma(){
  char date[10]="hh:mm:ss";
  rtc.now().toString(date);
  Serial.println(date);
  Serial.println(" SQW: ");
  Serial.println(digitalRead(alarmapin));
  Serial.println(alarms[1] [4]);
  if(stalarm==1){
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("ALARMA");
    lcd.setCursor(4, 0);
    lcd.print("ACTIVADA");
    Serial.println("alarma activada");
    digitalWrite(Alarmin, HIGH);
    delay(10000);
    rtc.clearAlarm(1);
    stalarm=0;
    Serial.println("Alarma desactivada");
    digitalWrite(Alarmin, LOW);
    c++;
  }
}
