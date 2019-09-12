// pinos dos motores
// aqui será usado uma ponte h para fazer a reversão do sentido de rotação do motor
#define motorEFrente  6
#define motorDFrente  10
#define motorETras 9
#define motorDTras 11

// pinos dos sensores ultrassônicos
#define echoE  2
#define trigE  3
#define echoD  4
#define trigD  5

float distanceE;
float distanceD;
long durationE, durationD;

// pinos dos sensores de linha
#define IRFrentePinE A0
#define IRTrasPin  A1
#define IRFrentePinD A2

int distance = 50; // essa variável será o valor padrão máximo para distância ao adversário. Serve para não detectar coisas fora do tatame.
int IRFrenteE; // variáveis que servirão para identificação da cor e evitar a queda do robo do tatame.
int IRTras;               
int IRFrenteD;
int tempo = 0;
int botao = 0;
int tempoFuga = 400;
int tempoAtaque = 200;
int ultimoTempoAtaque = 0;
long aleatorio=1;
int cont=0;
#define pinBotao 7

void andarFrente(int velocidade) { // não definir limites de parada, vamos fazer no loop (estratégia básica)
  analogWrite(motorEFrente, velocidade);
  analogWrite(motorDFrente, velocidade);
  analogWrite(motorETras, 0);
  analogWrite(motorDTras, 0);
  //Serial.println("andar frente");
}

void andarTras(int velocidade) {
  analogWrite(motorEFrente, 0);
  analogWrite(motorDFrente, 0);
  analogWrite(motorETras, velocidade);
  analogWrite(motorDTras, velocidade);
  //Serial.println("andar tras");
}

void girarEsquerda(int velocidade) {
  analogWrite(motorEFrente, velocidade);
  analogWrite(motorDFrente, 0);
  analogWrite(motorETras, 0);
  analogWrite(motorDTras, velocidade);
  //Serial.println("girar esquerda");
}

void girarDireita(int velocidade) {
  analogWrite(motorEFrente, 0);
  analogWrite(motorDFrente, velocidade);
  analogWrite(motorETras, velocidade);
  analogWrite(motorDTras, 0);
  //Serial.println("girar direita");
}

void procurar(int velocidade){
  if(cont <= 23)
    cont++;
  else{
    aleatorio = random(1,5);
    cont = 0;
  }
  if(aleatorio <= 1)
    girarDireita(velocidade);
  else if(aleatorio <= 2)
    girarEsquerda(velocidade);
  else if(aleatorio <= 3){
    andarFrente(velocidade);
    cont = cont + 3; //aumentamos mais o cont para não andar muito pra frente/tras, pois pode cair, girar não precisa porque ele roda parado
  }
  else{
    andarTras(velocidade);
    cont = cont + 3; 
  }
}

void conferir(int velocidade){
  if(distanceE < 80 && distanceD < 80){
    andarFrente(velocidade);
  }
  else if(distanceE < 80){
    girarEsquerda(100);
    girarEsquerda(100);
    andarFrente(100);
  }
  else{
    girarDireita(100);
    girarDireita(100);
    andarFrente(100);
  }
}

float lerUltraE() { //leitura da distancia com média ponderada entre 5 leituras
    digitalWrite(trigE, LOW);
    digitalWrite(trigE, HIGH);
    durationE = pulseIn(echoE, HIGH, 50000);
    distanceE = durationE * 0.0340 / 2; // Velocidade do som dividido por 2 já que é ida e volta.
    // equação horária do MRU: d = v*t.
    if(distanceE == 0 || distanceE>80)
      distanceE=80;
    Serial.print(distanceE);
    Serial.print(";");
    return distanceE;
}

float lerUltraD() {
  digitalWrite(trigD, LOW);
  digitalWrite(trigD, HIGH);
  durationD = pulseIn(echoD, HIGH, 50000);
  distanceD = durationD * 0.034 / 2; // Velocidade do som dividido por 2 já que é ida e volta.
  // equação horária do MRU: d = v*t.
  if(distanceD == 0 || distanceD>80)
      distanceD=80;
  Serial.print(distanceD);
  Serial.println(";");
  return distanceD;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(motorEFrente, OUTPUT);
  pinMode(motorDFrente, OUTPUT);
  pinMode(motorETras, OUTPUT);
  pinMode(motorDTras, OUTPUT);

  pinMode(echoE, INPUT);
  pinMode(trigE, OUTPUT);
  pinMode(echoD, INPUT);
  pinMode(trigD, OUTPUT);

  pinMode(IRFrenteE, INPUT);
  pinMode(IRTras, INPUT);
  pinMode(IRFrenteD, INPUT);

  randomSeed(analogRead(0)); // isso faz com que a sequência de números gerados sejam diferentes.
  
  while (botao == 0) { // ele só sai daqui quando o botão for precionado, indicando que deve começar a luta.
    botao = digitalRead(pinBotao);
  }
  delay(5000);  // tempo definido pela competição para começar a lutar após ser ligado: 5 segundos
}  

  enum ESTADO {Fugindo, Procurando, Atacando};
  ESTADO estado = Procurando;
  int ultimoTempo = 0;

void loop() {
  // put your main code here, to run repeatedly:

  lerUltraE();
  lerUltraD();
  IRFrenteE = analogRead(IRFrentePinE)-200; //vai de 0 a 1023, menor que 301 ele identifica branco. -200 porque o sensor estava com um offset.
  IRTras = analogRead(IRTrasPin);
  IRFrenteD = analogRead(IRFrentePinD);
  //Serial.println(IRFrenteE);
  //Serial.println(IRFrenteD);
  //Serial.println(IRTras);
  bool precisaAtacar = estado == Atacando && millis() - ultimoTempoAtaque < tempoAtaque;
  if (precisaAtacar) {
    andarFrente(255);
    estado = Atacando;
  }
  if (IRFrenteE < 301 && IRFrenteD < 301) { // a preocupação principal é não sair do tatame, então verificamos primeiro os sensores de linha
    andarTras(255);
    if(estado!=Fugindo)
    {
      estado = Fugindo;
      ultimoTempo = millis();
      //Serial.println("Fugindo");
    }
    //Serial.println("Os dois");
    //delay(tempoDelay);
  }
  else if (IRFrenteD < 301) { //aqui, vamos fazer uma movimentação especial, uma mistura de giro com andar pra tras.
    analogWrite(motorEFrente, 0);
    analogWrite(motorDFrente, 0);
    analogWrite(motorETras, 153);
    analogWrite(motorDTras, 255);
    if(estado!=Fugindo)
    {
      estado = Fugindo;
      ultimoTempo = millis();
      //Serial.println("Fugindo");
    }
    
    //Serial.println("Direito");
    //delay(tempoDelay);
  }
  else if (IRFrenteE < 301) {
    analogWrite(motorEFrente, 0);
    analogWrite(motorDFrente, 0);
    analogWrite(motorETras, 255);
    analogWrite(motorDTras, 153);
    //Serial.println("Esquerdo");
    if(estado!=Fugindo)
    {
      estado = Fugindo;
      ultimoTempo = millis();
      //Serial.println("Fugindo");
    }
    //delay(tempoDelay);
  }
  else if (IRTras < 301) {
    andarFrente(255);
    //Serial.println("tras");
    if(estado!=Fugindo)
    {
      estado = Fugindo;
      ultimoTempo = millis();
      //Serial.println("Fugindo");
    }
    //delay(tempoDelay);
  }
  // se não entra nos três primeiros, quer dizer que o robo está na região preta.
  else if (distanceD < distance && distanceE < distance && !precisaAtacar) { // adversário à frente
    andarFrente(255); // aqui ele ataca.
    //Serial.println("atacando");
    estado = Atacando;
    ultimoTempoAtaque = millis();
    delay(tempo);
  }
  else if (distanceD < distance && millis()-ultimoTempo>tempoFuga && !precisaAtacar) { // se isso for verdade, quer dizer que distanceE>distance.
    girarDireita(255);
    //Serial.println("alvo a direita");
    estado = Procurando;
    
    delay(tempo);
  }
  else if (distanceE < distance && millis()-ultimoTempo>tempoFuga && !precisaAtacar) { // se isso for verdade, quer dizer que distanceD>distance.
    girarEsquerda(255);
    estado = Procurando;
    //Serial.println("alvo a esquerda");
    
    delay(tempo);
  }
  /*else if(distanceE < 80 && distanceE > distance || distanceD < 80 && distanceD > distance){
    conferir(100);
  }*/
  else if(millis()-ultimoTempo>tempoFuga){ // se ele chegar nessa condição, quer dizer que as duas distancias são maiores que distance, então ele deve fazer algo para procurar o adversário.
    // aqui que podemos mudar para uma estratégia melhor/mais dinâmica
    procurar(100);
    estado = Procurando;
    //Serial.println("Procurando");
  }
}
