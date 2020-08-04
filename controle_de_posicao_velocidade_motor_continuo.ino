// Programa  elaborado  para  controle  de  velocidade  e  posicao
// de um motor de  corrente  continua . O programa  gera uma referencia
// rampa de  posicao com  inclinacao  definida  pela  variavel  inc.

#define  pos 1
#define  neg 0
#define  off  2

// PINAGEM ARDUINO:∗∗∗∗∗∗∗∗
//  os  pinos  2 e 3  sao  usados  para  interrupcao .
#define chA_MOTORA 2
#define chA_MOTORB 3
#define  fimA_PIN 4
#define  fimB_PIN 5
#define chB_MOTORA 6
#define chB_MOTORB 7

//L298n :
#define IN1 A0
#define IN2 A1
#define IN3 A2
#define IN4 A3
#define ENA 9
#define ENB 10

//∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗
// VARIAVEIS TIMER 2
unsigned  long  counterT2 = 0;

// OVERFLOW DA VARIAVEL EM 2^31−1∗16 = 9.54h 
float  T2res = 16;

//  us − cada  pulso  do  timer  equivale  a 16us(microsegundo)

//∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗
// VARIAVEIS GLOBAIS

float changeRes = 0.00799;        // 2∗pi/786
float risingRes = 0.01599;        // 2∗pi/393
float Tloop = 10000;              // tempo de amostragem  desejado em microsegundos .

//∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗
// VARIAVEIS DENTRO DO LOOP
unsigned long timeL , timer ;    //  para calculo de tempo  dentro  do  loop .
unsigned long  timeLaux ;
unsigned long OVFTL;             //  conta  overflow  atual
unsigned long OVFTLaux;          //  conta  overflow  an t er i or
unsigned long TL;                //  guarda  periodo  atual  do LOOP
float delayL = 0;                //  guarda  periodo  atual  do LOOP

//CONTROLADOR VELOCIDADE:
float erro_velAaux = 0;
float erro_velA = 0;
int PWM_Aaux = 0;
int PWM_A = 0;
float ref_velA = 0;               //  limites: 0.43rad/s a 3.2rad/s

//CONTROLADOR POSICAO:
float erro_posAaux2 = 0;
float erro_posAaux = 0;
float erro_posA = 0;
float  ref_velAaux2 = 0;
float  ref_velAaux = 0;
float  ref_posA ;                 //  referencia  de  posicao  a  ser  seguida em rad .
float  final_posA ;
float  final_posAaj = 0;
float  ref_posAaux ;
float  inc ;
float  posA ;

//∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗∗
// MOTOR A
unsigned  long  timeA ;           //  para  calculo  de tempo e  frequencia .
unsigned  long  timeAaux ;
unsigned  long OVFTA;             //  conta  overflow  atual
unsigned  long OVFTAaux;          //  conta  overflow  an t er i or
unsigned  long Ta ;               //  guarda  periodo  atual  do  s i n a l  no motor A
unsigned  long Ta_aux ;
long  counterA = 0;
long  counterAaux = 0;
float velA = 0;
int  chA = 0;
int  chB = 0  ;
int  chAaux = 0;
int  flagA = 1;                   // indica  direcao  de  rotacao ;


//INICIO DO PROGRAMA

void  countA ();
void L298N_MOTORA( int dir, int pwm);

//TIMER2 PARA TER TEMPO MAIS PRECISO

ISR(TIMER2_OVF_vect){
  counterT2++;                      //  conta  quantos  overflows  desde  o  inicio  do programa .
}
void  setup () {
  Serial.begin(115200);
  pinMode (IN1 , OUTPUT) ;
  pinMode (IN2 , OUTPUT) ;
  pinMode (IN3 , OUTPUT) ;
  pinMode (IN4 , OUTPUT) ;
  pinMode (13 , OUTPUT) ;
  attachInterrupt (0 ,  countA , CHANGE) ;  //INICIA O LOOP APENAS QUANDO ENVIA  ’A’ PELA SERIAL.  teste  de  conexao.
  digitalWrite (13 ,HIGH) ;
  Serial.println (’a’) ;
  char  a = ’b ’ ;
  
  while ( a != ’a’ ) {
    a = Serial.read();
  }
  
  digitalWrite (13 ,LOW) ;          //  teste  de  conexao  visual - liga led imbutido na placa
  TCCR2A = 0x00 ;                   //  Timer operando em modo normal
  TCCR2B = 0x06 ;                   //  Prescaler  1:256−resolucao  de 16us
  TIMSK2 = 0x01 ;                   //  Habilita  interrupcao  do Timer2

  
  //  inicializacao  de  variaveis :
  
  chAaux = digitalRead (chA_MOTORA) ;
  Ta = 10000000000;
  velA = 0;
  posA = 0;
  erro_posAaux2 = 0;
  erro_posAaux = 0;
  ref_velAaux = 0;
  ref_posA = 0;                       // referencia  inicial
  ref_posAaux = 0;
  final_posA = 3.1416;                // posicao  final  desejada
  inc = 1.5;                          // inclinacao  da rampa  fixa
  ref_velA = 0;
  final_posA = 3.1415;
  final_posA = round ( final_posA/changeRes )∗changeRes ;
  counterT2 = 0;//  zera  contador  de  overflows
  TCNT2 = 0x00 ;//  zera  contadortime
  Aaux = TCNT2;
  OVFTAaux = counterT2 ;
}
 
void  loop ()  {
  OVFTLaux = counterT2 ;
  timeLaux = TCNT2;
  
  ///////////////////////////////////////////
  // calculo  da  velocidade  atual :
  if ( flagA == 1)  
    velA = changeRes /(Ta∗0.000001);
  else  
    velA = (0.0−changeRes )/(Ta∗0.000001);
    
  Serial.println(velA,4);
  posA = counterA∗changeRes ;           //  verifica  posicao  angular  atual
  
  // geracao  de  referencia  degrau.
  if (posA != final_posA ){
    if ( ref_posA != final_posA ){
      if ( final_posA >= posA ){
        ref_posA = ref_posAaux + inc∗T;
        ref_posA = round( ref_posA/changeRes )∗changeRes ;
      }
      e l s e {
        ref_posA = ref_posAaux−inc∗T;
        ref_posA = round ( ref_posA/changeRes )∗changeRes ;
      }
    }
  } e l s e {
    ref_posAaux = posA ;
    ref_posA = posA ;
  }
  
  ref_posAaux = ref_posA ;
  Serial.println( ref_posA , 4 ) ;
  Serial.println(posA , 4 ) ;
  // Serial.println( counterA ) ;
  
  //CONTROLE PID DE POSICAO MOTOR A:
  erro_posA = ref_posA−posA ;
  ref_velA = 170.1∗( erro_posA−1.88∗erro_posAaux + 0.8824∗erro_posAaux2 ) + ref_velAaux ;
  erro_posAaux2 = erro_posAaux ;
  erro_posAaux = erro_posA ;
  ref_velAaux = ref_velA ;
  controle_vel ( ) ;
  OVFTL = counterT2 ;
  timeL = TCNT2;                        
  
  //  periodo  instantaneo em microssegundos :
  TL = ((OVFTL−OVFTLaux)∗255+timeL−timeLaux )∗T2res ;
  delayL = Tloop−TL;                          //  calcula  delay  para  que tempo de amostragem  seja  10ms
  delayMicroseconds( delayL );
  // Serial.println(TL+delayL , 5 );
}

//FUNCAO PARA CONTAGEM DE PULSOS NO MOTOR A
void  countA (){
  OVFTA = counterT2 ;
  timeA = TCNT2;
  noInterrupts();
  Ta = ((OVFTA−OVFTAaux)∗255+timeA−timeAaux )∗T2res ;  //  periodo  instantaneo  
  usi n t e r r u p t s ( ) ;
  timeAaux = timeA ;
  OVFTAaux = OVFTA;
  chB = digitalRead(chB_MOTORA) ;
  chA = digitalRead(chA_MOTORA) ;
  
  if (chB != chAaux){
    counterA−−;
    flagA = 0;
  } else {
    counterA++;
    flagA = 1;
  }
  
  chAaux = chA ;          //  guarda  ultimo  estado  do  canal A do  encoder .
}

//Acionamento do motor :
void L298N_MOTORA( int  dir ,  int pwm){
  
  if( dir == 0){
    // digitalWrite (IN1 ,LOW) ;
    // digitalWrite (IN2 ,HIGH) ;
    PORTC &= ~(1<<PORTC0) ;
    PORTC |= (1<<PORTC1) ;
  }else  if ( dir == 1){
    // digitalWrite (IN1 ,HIGH) ;
    // digitalWrite (IN2 ,LOW) ;
    PORTC |= (1<<PORTC0) ;
    PORTC &= ~(1<<PORTC1) ;
  } else {                    // usado  para  erro  ou  desligar  o motor
    // digitalWrite (IN1 ,LOW) ;
    // digitalWrite (IN2 ,LOW) ;
    PORTC &= ~(1<<PORTC0) ;
    PORTC &= ~(1<<PORTC1) ;
  }
  
  analogWrite (ENA,pwm) ;
}


void  controle_vel (){
  
  //CONTROLE DE VELOCIDADE MOTOR A:
  erro_velA = ref_velA−velA;
  PWM_A = round(46.808∗( erro_velA−0.8495∗erro_velAaux ) + PWM_Aaux) ;
  erro_velAaux = erro_velA ;
  PWM_Aaux = PWM_A;
  
  //ENVIO DO PWM PARA O MOTOR A
  if( ref_velA >= 0){
    if(PWM_A > 255){
      L298N_MOTORA( pos , 255 );
    } else  if (PWM_A > 0){
      L298N_MOTORA( pos ,PWM_A) ;
    }else {
      L298N_MOTORA( off , 0 ) ;
    }
  }else {
    if (PWM_A <−255){
      L298N_MOTORA( neg , 2 5 5 ) ;
    }else  if (PWM_A < 0){
      L298N_MOTORA( neg , abs (PWM_A) ) ;
    }else {
      L298N_MOTORA( off , 0 ) ;
    }
  }
}
