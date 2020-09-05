// Identificación del dispositivo. Para que sea único, pongo en el nombre la fecha de fabricación.
#define DEVICE_ID "NODEMCU-20191215"
#define DEVICE_SSID "TOTEESPCFG20191215"   // El SSID debe ser único, pongo la fecha de fabricación (15/12/2019) como GUID.

// Si se habilita el LEVEL_2, habilitar también en LEVEL_1

// #define _TOTE_DEBUG_LEVEL_1_ON_   // Descomentar para mostrar info de debug de primer nivel por serial
// #define _TOTE_DEBUG_LEVEL_2_ON_   // Descomentar para mostrar info de debug de segundo nivel por serial

// Macros para facilitar la salida de información por el Serial.
#ifdef _TOTE_DEBUG_LEVEL_1_ON_
#define _TOTE_DEBUG_LEVEL_1_(type, text) Serial.print("("); Serial.print(millis()); Serial.print(" millis)"); Serial.print(" ["); Serial.print(type); Serial.print("] "); Serial.println(text);
#define _TOTE_DEBUG_LEVEL_1_VALUE_(type, text, value) Serial.print("("); Serial.print(millis()); Serial.print(" millis)"); Serial.print(" ["); Serial.print(type); Serial.print("] "); Serial.print(text); Serial.println(value);
#else
#define _TOTE_DEBUG_LEVEL_1_(type, text) void();
#define _TOTE_DEBUG_LEVEL_1_VALUE_(type, text, value) void();
#endif

#ifdef _TOTE_DEBUG_LEVEL_2_ON_
#define _TOTE_DEBUG_LEVEL_2_(type, text) Serial.print("("); Serial.print(millis()); Serial.print(" millis)"); Serial.print(" ["); Serial.print(type); Serial.print("] "); Serial.println(text);
#define _TOTE_DEBUG_LEVEL_2_VALUE_(type, text, value) Serial.print("("); Serial.print(millis()); Serial.print(" millis)"); Serial.print(" ["); Serial.print(type); Serial.print("] "); Serial.print(text); Serial.println(value);
#else
#define _TOTE_DEBUG_LEVEL_2_(type, text) void();
#define _TOTE_DEBUG_LEVEL_2_VALUE_(type, text, value) void();
#endif



/*

	PINOUT DEL NodeMCU Mini D1

	Los includes de ESP8266Wifi.h y similares redefinen las constantes de los pines de Arduino para que coincidan con el pinout
	de las placas NodeMCU, por lo que se pueden usar en el código sin mayor problema.

	Ciertos pines de la placa están conectados a funciones activas del ESP8266, por lo que no se deben usar como pines de E/S, por ejemplo
	TX, RX, etc.

	Esta es la equivalencia entre los pines de la placa NodeMCU (Mimi D1) y los pines del ESP8266.

	Pin		Function								ESP - 8266 Pin
	---		-------------------------------------	--------------

	TX		TXD										TXD
	RX		RXD										RXD
	A0		Analog input, max 3.3V input			A0
	D0		IO										GPIO16
	D1		IO, SCL									GPIO5
	D2		IO, SDA									GPIO4
	D3		IO, 10k Pull - up				       	GPIO0
	D4		IO, 10k pull - up, BUILTIN_LED	        GPIO2
	D5		IO, SCK									GPIO14
	D6		IO, MISO								GPIO12
	D7		IO, MOSI								GPIO13
	D8		IO, 10k pull - down, SS					GPIO15
	G		  Ground 								GND
	5V		5V										5V
	3V3		3.3V									3.3V
	RST		Reset									RST



	Placa NodeMCU .

	IMPORTANTE: Evito usar el D3 (GPIO0) porque tiene significado especial para el arranque del dispositivo.
*/


// Configuración de los pines de NodeMCU
#define AP_CONF_PIN       D2    // Para entrar en modo de selección del AP.
#define LED_WIFI_PIN      D5    // Pin para el led que indica conexión con el AP.		
#define RELE_SGN_PIN      D6	// Pin para excitar la bobina del relé.


/*
	Cuando se inicia el dispositivo, entra en un bucle de espera para dar posibilidad de pulsar D2 (GPIO4). Este tiempo viene dado por el
	define CONFIG_WINDOW. El led parpadeará rápido durante este tiempo. Si se pulsa, emite un AP con el SSID dado por su correspondiente
	definición en el código, así como el PASS.

	ESP8266 emite un AP a cuya red debemos conectarnos y monta un servidor http para facilitar la configuración al AP
	verdadero. la IP de este servidor es 192.168.4.1 y nos podemos conectar con un móvil, por ejemplo. Una vez configurado, el
	dispositivo se reinicia. Debemos esperar sin hacer nada a que pase de nuevo el periodo de configuración.

	Ahora se conecta al AP correcto y después al servidor MQTT en 192.168.1.200

	Cuando el dispositivo pierde la conexión con el AP, por ejemplo, debido a que éste se ha caido o un corte eléctrico, el ESP8266 se reiniciará
	comenzando de nuevo todo el procedimiento.

 */



// Damos un margen de 2 segundos para que los leds parpadeen.
#define CHECKLED_WINDOW 2000UL

// Damos un margen de 10 segundos para pulsar el botón que lanza el portal de configuración.
#define CONFIG_WINDOW 10000UL  


 // Librerías del framework (Arduino-Espressif8266) que usaré.
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>            // Servidor DNS que se usa para redirigir todas las request al portal de configuración.
#include <ESP8266WebServer.h>     // Servidor web local que muestra el portal de configuración.
#include <EEPROM.h>
#include <time.h>

// Librerías clonadas desde Git. 
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Librerias_desde_Git\WiFiManager\WiFiManager.cpp"
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Librerias_desde_Git\pubsubclient\src\PubSubClient.cpp"
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Librerias_desde_Git\Time\Time.cpp"


// Librerías propias. Se encuentran disponibles en https://github.com/MisLibrerias
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Mis_Librerias\ToteDebouncedBtn\ToteDebouncedBtn.cpp"
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Mis_Librerias\ToteBlinkOutputLed\ToteBlinkOutputLed.cpp"
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Mis_Librerias\ToteAsyncDelay\ToteAsyncDelay.cpp"
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Mis_Librerias\ToteAnalogSensor\ToteAnalogSensor.cpp"


// IMPORTANTE. VER INFORMACIÓN SOBRE EL WDT en la clase ToteESPMillisDelay
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Mis_Librerias\ToteESPMillisDelay\ToteESPMillisDelay.cpp"


 /************************************
  * PROTOTIPO DE FUNCIONES           *
  ************************************/
 
void checkLeds(void);
void doWaitForConfig(void);
void doConnectSSID(void);
void doNormalRun(void);
boolean checkWiFi(void);
void firstRun(void);
void eepromLoadData(void);
void eepromResetData(void);
void eepromUpdateData(void);
void showEEPROMData(void);
void mqttReconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttCheckConnectionStatusCallback(void);
void sendTimerState(boolean theState);
void sendPowerOnStatusCallback(void);
void sendRelayCommand(uint8_t theCommand);
void relayAutomaticOnOffTimerCallback(void);
void sendTestResponse(void);
void wifiCheckSignalStrengthCallback(void);
void refheshNoderedDashboardsCallback(void);


  /********************************
   * OBJETOS Y VARIABLES GLOBALES *
   ********************************/

unsigned int initialMillis;	// Almacena los milis desde el inicio del programa.
boolean isPanicked = false; // Sirve para indicar si el ESP ha perdido la conectividad con la WiFi.
long wifiSignal = 0; 		// Almacena la intensidad de la señal Wifi recibida (RSSI) en dBm (Actualizada en loop)


const char* SSID = DEVICE_SSID;
const char* PASS = "DarthVader*1";


// Muy importante. https://pubsubclient.knolleary.net/api.html#configoptions
#define MAX_MQTT_CHARS 100   // longitud de los arrays para procesar mensajes MQTT.

char mqttLastStateOpTxt[MAX_MQTT_CHARS];  // Almacena mensaje sobre la última operación con el servidor MQTT.

const char* MQTT_SERVER = "192.168.1.200";  			// IP del servidor MQTT
#define MQTT_CHECK_CONNECTION_STATUS_INTERVAL  10000UL	// Si se pierde la conexión con el servidor MQTT se reintentará pasado este tiempo.

const char* mqttClientID = DEVICE_ID;   // El id debe ser único, pongo la fecha de fabricación como GUID.

// Objeto para realizar 'delays' alimentando al WDT de software del ESP8266 para evitar los resets.
ToteESPMillisDelay myESPDelay = ToteESPMillisDelay(2000);

// Instancio objeto WiFiManager que permitirá mostrar el portal de configuración de acceso al AP.
WiFiManager myWiFiManager;

// Instancio objeto  auxiliar para PubSubClient
WiFiClient myWiFiClient;

// Instancio objeto para protocolo MQTT.
PubSubClient myMqttClient(myWiFiClient);

// Instancio un objeto ToteDebounceBtn para quitar el rebote al switch AP_CONF_PIN.
ToteDebouncedBtn  myAPConfigRelaySwitchBtn = ToteDebouncedBtn(AP_CONF_PIN, NULL);

// Sirve para indicar el estado en el que se encuentra el dispositivo en 'loop()'
enum LOOP_RUN_MODE { CHECK_LEDS, WAIT_FOR_CONFIG, CONNECT_SSID, NORMAL_RUN };

// Modo actual de funcionamiento del programa en la función 'loop()'
int myLoopRunMode;

// Instancio un objeto BlinkOutputLed que indica el estado de la conexión con el AP.
ToteBlinkOutputLed myWiFiLed(100UL, LED_WIFI_PIN, ToteBlinkOutputLed::FINITE_BLINK, NULL); // Será obligatorio llamar a su método 'init' en 'setup'.

// Temporizador de indicará cuando hay que testear la conexión con el servidor MQTT.
ToteAsyncDelay myMQTTCheckConnectionStatusTimer(MQTT_CHECK_CONNECTION_STATUS_INTERVAL, mqttCheckConnectionStatusCallback);

// Instancio un objeto temporizador que indicará cuando enviar el estado de encendido del relé al Broker MQTT.
#define SEND_POWER_ON_STATUS_INTERVAL 5000UL
ToteAsyncDelay myPowerOnStatusTimer(SEND_POWER_ON_STATUS_INTERVAL, sendPowerOnStatusCallback);

// Indica si se ha recibido hora desde el topic de NodeRed.
bool isValidTime = false;
time_t theTime;

// Configuración del temporizador de encendido automático.
#define DEFAULT_TIMER_ENABLED_MODE	0		// Modo por defecto del temporizador de encendido, 1 = encendido, 0 = apagado.
#define DEFAULT_TIMER_H_INIT		20		// Hora por defecto a la que se inicia el temporizador.
#define DEFAULT_TIMER_M_INIT		0		// Minutos (de la hora anterior) a la que se inicia el temporizador.
#define DEFAULT_TIMER_DURATION      120		// Duración por defecto del temporizador.
#define NO_VALID_DAY				0       // Usado por el algoritmo del temporizador para detectar el estado no inicializado. Poner un valor fuera del entorno [1,31]
#define AUTOMATIC_ON_OFF_INTERVAL	2000UL  // polling de 2 segundos.

uint8_t timerDay = NO_VALID_DAY;

#define DASHBOARD_REFRESH_INTERVAL 2500UL	// Tiempo (en ms) para refrescar el Dashboard de Node-Red.
#define WIFI_CHECK_SIGNAL_STRENGTH 5000UL	// Tiempo (en ms) para medir la señal de la Wifi.


// Este temporizador se llama para ver si hay que encender las luces de forma automatica.
ToteAsyncDelay myRelayAutomaticOnOffTimerCallback(AUTOMATIC_ON_OFF_INTERVAL, relayAutomaticOnOffTimerCallback);

// Indica el estado de encendido del relé
bool isRelayON = false;

// Sirve para indicar el comando de estado al relé.
enum RELAY_COMMAND { RELAY_POWER_ON, RELAY_POWER_OFF, RELAY_SWITCH_POWER };

// Este temporizador mide la intensidad de la señal WiFi.
ToteAsyncDelay myWiFiCheckSignalStrength(WIFI_CHECK_SIGNAL_STRENGTH, wifiCheckSignalStrengthCallback);

// Temporizador para actualizar los objetos de los Dashboards.
ToteAsyncDelay myNodeRedDashboardRefreshTimer(DASHBOARD_REFRESH_INTERVAL, refheshNoderedDashboardsCallback);

// Creo una estructura que servirá para almacenar y recuperar datos en la EEPROM
struct {
	uint8_t		EEPROM_MAGIC_NUMBER;				// Truco, amaceno el valor 69 si ya se ha inicializado los valores de la eeprom al menos una vez.
	uint8_t		EEPROM_TIMER_ENABLED_UINT8_T;		// Indica si el temporizador está activado.
	uint8_t		EEPROM_TIMER_H_INIT_UINT8_T;		// Hora a la que se activa el temporizador.
	uint8_t		EEPROM_TIMER_M_INIT_UINT8_T;		// Minutos (de la hora anterior) a la que se activa el temporizador.
	uint16_t	EEPROM_TIMER_DURATION_UINT16_T; 	// Duración del temporizador en minutos (max 1440 o 24 horas)
} myEEPROMData;


/*********************************************************
 * FIN DE LA DECLARACIÓN DE OBJETOS Y VARIABLES GLOBALES *
 *********************************************************/


void setup() {

#if defined(_TOTE_DEBUG_LEVEL_1_ON_) || defined (_TOTE_DEBUG_LEVEL_2_ON_)
	// Configuramos la conexión serie
	Serial.begin(115200);
#endif

	// Defino los pines 'RESET_PIN' y 'POWER_ON_PIN' como salidas. Los demás pines lo inicializan los respectivos objetos.
	pinMode(RELE_SGN_PIN, OUTPUT);

	// Indicamos al objeto EEPROM cuántos bytes vamos a necesitar.
	EEPROM.begin(sizeof(myEEPROMData));

	// Cargamos los valores de la EEPROM en la estructura.
	eepromLoadData();

	// Intento leer el valor mágico 69 de la EEPROM. si no lo leo, entonces la reseteo para almacenar los valores por defecto.
	if (myEEPROMData.EEPROM_MAGIC_NUMBER != 69)
		eepromResetData();

	// Evitamos que muerda el perro.
	yield();

	// Inicialización del objeto MQTT
	myMqttClient.setServer(MQTT_SERVER, 1883);
	myMqttClient.setCallback(mqttCallback);

	// Inicializamos el objeto que gestiona el LED WiFi.
	myWiFiLed.init();

	// Inicializamos el objeto que gestiona el botón para entrar en modo de configuración de AP.
	myAPConfigRelaySwitchBtn.init();

	// Vamos a dar un segundo para que se estabilice todo.
	myESPDelay.stop(1000UL);

	// Limpiamos un poco la ventana del monitor serie.
	_TOTE_DEBUG_LEVEL_1_("setup()", "--------------------------------------------------------------------------");
	_TOTE_DEBUG_LEVEL_1_("setup()", "              INICIANDO LA EJECUCION DEL CODIGO");
	_TOTE_DEBUG_LEVEL_1_("setup()", "--------------------------------------------------------------------------");

	// Evitamos que muerda el perro.
	yield();

	// Configuración inicial del gadget leyendo valores almacenados en EEPROM.
	firstRun();
	
	// Activo el parpadeo de los led, para comprobar que no están fundidos.
	myWiFiLed.setBlinkType(ToteBlinkOutputLed::INFINITE_BLINK);
	myWiFiLed.ledON();

	// Entramos en loop() en modo de chequeo de leds.
	myLoopRunMode = LOOP_RUN_MODE::CHECK_LEDS;

	// Iniciamos el temporizador
	initialMillis = millis();

	_TOTE_DEBUG_LEVEL_1_("setup", "Entrando en loop()");
}

void loop() {
	switch (myLoopRunMode) {
	case CHECK_LEDS:
		checkLeds();
		break;

	case WAIT_FOR_CONFIG:
		doWaitForConfig();
		break;

	case CONNECT_SSID:
		doConnectSSID();
		break;

	case NORMAL_RUN:
		doNormalRun();
		break;
	}

	// Evitamos que muerda el perro.
	yield();

	// Actualizo objetos que lo necesitan en cada iteración del loop().
	myWiFiLed.check();

	// Compruebo si se ha pulsado el botón en este caso se interpreta como una conmutación del Relé.
	if (myAPConfigRelaySwitchBtn.check()) {
		_TOTE_DEBUG_LEVEL_1_("doWaitForConfig", "Se ha pulsado el boton para conmutar el rele.");

		// Conmuto el Relé.
		sendRelayCommand(RELAY_COMMAND::RELAY_SWITCH_POWER);
	}
}

void checkLeds(void) {
	// Esta función realiza una simple espera para dar tiempo a que parpadeen varias veces los leds.
	if (initialMillis + CHECKLED_WINDOW > millis())
		return; // Me quedo en checkLeds sin avanzar durante CHECKLED_WINDOW segundos

	// Cuando llegue aquí es porque ya han pasado el tiempo de parpadeo.
	// Apago los leds.
	myWiFiLed.ledOFF();

	_TOTE_DEBUG_LEVEL_1_("checkLeds", "Comprobacion finalizada. Entrando en ventana de configuracion de AP.");

	// Iniciamos el temporizador para el siguiente estado.
	initialMillis = millis();

	// Pongo el led Wifi a parpadear.
	myWiFiLed.setBlinkType(ToteBlinkOutputLed::INFINITE_BLINK);
	myWiFiLed.ledON();

	// Pasamos al siguiente estado.
	myLoopRunMode = LOOP_RUN_MODE::WAIT_FOR_CONFIG;
}

void doWaitForConfig(void) {
	// Ventana de lanzamiento del portal de configuración.
	// Compruebo si se ha pulsado el botón o si la WiFi está desconectada.
	if (myAPConfigRelaySwitchBtn.check()) {
		_TOTE_DEBUG_LEVEL_1_("doWaitForConfig", "Se ha pulsado el boton para lanzar el portal de configuracion.");

		// Se ha pulsado. Invocamos al portal de configuración
		// Creamos el AP con la siguiente configuración.
		myWiFiManager.startConfigPortal(SSID, PASS);

		// Mientras el ESP está en modo AP, nos conectamos a él con un navegador en la IP 192.168.4.1, 
		// configuramos la WIFI, la guardamos y reiniciamos el dispositivo para que se conecte al AP deseado,
		// ya sin pulsar el botón de configuración

		// Cambiamos el modo de ejecución.
		myLoopRunMode = LOOP_RUN_MODE::NORMAL_RUN;

		// Congelo la animación del led y lo apago ya que se ha terminado el proceso de configuración con el AP.
		myWiFiLed.setBlinkType(ToteBlinkOutputLed::NO_BLINK);
		myWiFiLed.ledOFF();

		// Salimos.
		return;
	}

	// Compruebo si ha terminado el periodo de configuración en cuyo caso cambiamos el modo de ejecución.
	if (initialMillis + CONFIG_WINDOW < millis()) {
		_TOTE_DEBUG_LEVEL_1_("doWaitForConfig", "Se ha acabado el tiempo de espera para lanzar el portal de configuracion");


		// Congelo la animación del led y lo apago ya que se ha terminado el proceso de configuración con el AP y Thingerio.
		myWiFiLed.setBlinkType(ToteBlinkOutputLed::NO_BLINK);
		myWiFiLed.ledOFF();

		// Cambiamos el modo de ejecución.
		myLoopRunMode = LOOP_RUN_MODE::CONNECT_SSID;
	}
}

void doConnectSSID(void) {
	// Compruebo si la WiFi sigue conectada
	if (checkWiFi()) {
		// Cambiamos el modo de ejecución.
		myLoopRunMode = LOOP_RUN_MODE::NORMAL_RUN;

		_TOTE_DEBUG_LEVEL_1_VALUE_("doConnectSSID", "Conectado a la WiFi con la IP: ", WiFi.localIP());

		// Si hay WIFI entonces inicializo el objeto cliente NTP
		_TOTE_DEBUG_LEVEL_1_("doConnectSSID", "Inicializando objeto cliente NTP.");
	}
}

void doNormalRun(void) {
	// Esta función contiene los procedimientos que deben llamarse en cada iteración del loop() cuando la Wifi funciona y está conectada.
	// El el resto del loop() pongo las acciones que debe ejecutarse siempre.

	// Evitamos que muerda el perro.
	yield();

	// Comprobación de que el AP esté OK.
	if (checkWiFi()) {
		// AQUÍ HAY QUE PONER LAS LLAMADAS A UPDATE PARA LOS OBJETOS QUE NECESITAN DE CONECTIVIDAD A LA RED
		// EL RESTO PUEDE PONERSE EN EL PROPIO LOOP.

		// Compruebo que el cliente de publicación/subscripción de MQTT está conectado.
		myMQTTCheckConnectionStatusTimer.check();

		// Compruebo si hay mensajes que enviar al servidor MQTT.
		myPowerOnStatusTimer.check();

		// Obligatorio para que se procesen los mensajes
		myMqttClient.loop();

		// Compruebo si se ha cumplido el intervalo para comprobar el temporizador de encendido y apagado automático.
		myRelayAutomaticOnOffTimerCallback.check();

		// Compruebo si se ha cumplido el intervalo para medir la intensidad de la señal WiFi.
		myWiFiCheckSignalStrength.check();

		// Compruebo si es el momento de refrescar el Dashboard de Node-Red.
		myNodeRedDashboardRefreshTimer.check();
	}
}

boolean checkWiFi(void) {
	// Compruebo si la WiFi sigue conectada
	if (!WiFi.isConnected()) {
		// Tenemos un problema. 
		_TOTE_DEBUG_LEVEL_1_("doNormalRun", "WiFi desconectada. Entrando en ventana de autoconfiguracion de AP.");

		// Pongo el led a parpadear para indicar que se ha caido el AP.
		myWiFiLed.setBlinkType(ToteBlinkOutputLed::INFINITE_BLINK);
		myWiFiLed.ledON();

		// Cambiamos al modo de ejecución para dar la posibilidad de configurar un AP diferente.
		initialMillis = millis(); // Para que empiece la cuenta de nuevo.
		myLoopRunMode = LOOP_RUN_MODE::WAIT_FOR_CONFIG;

		// Indico que la WIFI se ha caido o no conecta.
		return false;
	}
	else {
		// Está conectada. Dejo el led Wifi encendido.
		myWiFiLed.setBlinkType(ToteBlinkOutputLed::NO_BLINK);
		myWiFiLed.ledON();

		// Indico que la WIFI está OK.
		return true;
	}
}

void firstRun(void) {
	// Función auxiliar. Recupera de la EEPROM los valores de configuración del dispositivo elegidos por el usuario.
	// Posteriormente llama a 'setAnimation' para iniciar la animación.
	// Leo 'myEEPROMData.EEPROM_ANIMATION_ID_UNIT8_T' almacenado en EEPROM
	eepromLoadData();
}

void eepromLoadData(void) {
	_TOTE_DEBUG_LEVEL_1_("eepromLoadData", "Leyendo datos de la EEPROM");

	EEPROM.get(0, myEEPROMData);
}

void eepromResetData(void) {
	// Función auxiliar. Guarda en EEPROM valores originales.
	// Almaceno animación por defecto en la EEPROM
	myEEPROMData.EEPROM_MAGIC_NUMBER = 69; // Esto indicará que la EEPROM se inicializá al menos una vez.
	myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T = DEFAULT_TIMER_ENABLED_MODE;
	myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T = DEFAULT_TIMER_H_INIT;
	myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T = DEFAULT_TIMER_M_INIT;
	myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T = DEFAULT_TIMER_DURATION;

	_TOTE_DEBUG_LEVEL_1_("eepromResetData", "guardando datos de la EEPROM");

	// Guardamos en EEPROM
	EEPROM.put(0, myEEPROMData);
	EEPROM.commit();
}

void eepromUpdateData(void) {
	_TOTE_DEBUG_LEVEL_1_("eepromUpdateData", "Actualizando datos en la EEPROM");

	// Guardamos en EEPROM
	EEPROM.put(0, myEEPROMData);
	EEPROM.commit();
}

void showEEPROMData(void) {
	_TOTE_DEBUG_LEVEL_1_VALUE_("showEEPROMData", "EEPROM_MAGIC_NUMBER = ", myEEPROMData.EEPROM_MAGIC_NUMBER);
	_TOTE_DEBUG_LEVEL_1_VALUE_("showEEPROMData", "EPROM_TIMER_ENABLED_UINT8_T = ", myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T);
	_TOTE_DEBUG_LEVEL_1_VALUE_("showEEPROMData", "EEPROM_TIMER_H_INIT_UINT8_T = ", myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T);
	_TOTE_DEBUG_LEVEL_1_VALUE_("showEEPROMData", "EEPROM_TIMER_M_INIT_UINT8_T = ", myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T);
	_TOTE_DEBUG_LEVEL_1_VALUE_("showEEPROMData", "EEPROM_TIMER_DURATION_UINT16_T = ", myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T);
}

void mqttReconnect() {
	_TOTE_DEBUG_LEVEL_1_("mqttReconnect", "Intentando reconectar con servidor MQTT...");

	// Intento la conexión.		// Attempt to connect
	if (myMqttClient.connect(mqttClientID)) {
		_TOTE_DEBUG_LEVEL_1_("mqttReconnect", "Conectado al servidor MQTT!!!");

		// Una vez conectado nos volvemos a subscribir.
		myMqttClient.subscribe("casa/time_source");
		myMqttClient.subscribe("casa/" DEVICE_ID "/timer_enabled");
		myMqttClient.subscribe("casa/" DEVICE_ID "/timer_initial_hours");
		myMqttClient.subscribe("casa/" DEVICE_ID "/timer_initial_minutes");
		myMqttClient.subscribe("casa/" DEVICE_ID "/timer_duration");
		myMqttClient.subscribe("casa/" DEVICE_ID "/masterSwitch");
	}
	else {
		_TOTE_DEBUG_LEVEL_1_("mqttReconnect", "Error al conectar al servidor MQTT");
	}
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
	// Array para procesar el payload.
	static char mqttPayload[MAX_MQTT_CHARS];

	unsigned int i;
	for (i = 0; i < length && i < MAX_MQTT_CHARS - 1; i++) {
		mqttPayload[i] = (char)payload[i];
	}
	mqttPayload[i] = '\0';


	_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "Ha llegado un mensaje para el topic: ", topic);
	_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "con el valor: ", mqttPayload);
	_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "y longitud : ", length);

	/**********************************
	*  PROCESADO DE LOS MENSAJES MQTT *
	***********************************/

	// casa/time_source
	if (strcmp(topic, "casa/time_source") == 0) {
		_TOTE_DEBUG_LEVEL_1_("mqttCallback", "Detectado topic casa/time_source");

		// Convierto topic a UnixEpoch
		theTime = atol(mqttPayload);

		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "theTime: ", theTime);

		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "day: ", day(theTime));
		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "month: ", month(theTime));
		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "year: ", year(theTime));
		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "hour: ", hour(theTime));
		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "minute: ", minute(theTime));
		_TOTE_DEBUG_LEVEL_2_VALUE_("mqttCallback", "Second: ", second(theTime));

		// Indico que la hora ya es válida.
		isValidTime = true;

		// Salimos.
		return;
	}

	// Evitamos que muerda el perro.
	yield();

	// casa/DEVICE_ID/timer_enabled
	if (strcmp(topic, "casa/" DEVICE_ID "/timer_enabled") == 0) {
		_TOTE_DEBUG_LEVEL_1_("mqttCallback", "Detectado topic casa/" DEVICE_ID "/timer_enabled");

		// Actualizo el valor de activación del temporizador.
		myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T = (uint8_t)atoi(mqttPayload);

		_TOTE_DEBUG_LEVEL_1_VALUE_("mqttCallback", "myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T ha cambiado a :", myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T);

		// Actualizo variables globales en la EEPROM.
		eepromUpdateData();

		// Salimos.
		return;
	}

	// Evitamos que muerda el perro.
	yield();

	// casa/DEVICE_ID/timer_initial_hours
	if (strcmp(topic, "casa/" DEVICE_ID "/timer_initial_hours") == 0) {
		_TOTE_DEBUG_LEVEL_1_("mqttCallback", "Detectado topic casa/" DEVICE_ID "/timer_initial_hours");

		// Actualizo el valor la hora de inicio del temporizador.
		myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T = (uint8_t)atoi(mqttPayload);

		_TOTE_DEBUG_LEVEL_1_VALUE_("mqttCallback", "myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T ha cambiado a :", myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T);

		// Actualizo variables globales en la EEPROM.
		eepromUpdateData();

		// Salimos.
		return;
	}


	// Evitamos que muerda el perro.
	yield();

	// casa/DEVICE_ID/timer_initial_minutes
	if (strcmp(topic, "casa/" DEVICE_ID "/timer_initial_minutes") == 0) {
		_TOTE_DEBUG_LEVEL_1_("mqttCallback", "Detectado topic casa/" DEVICE_ID "/timer_initial_minutes");

		// Actualizo el valor los minutos de inicio del temporizador.
		myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T = (uint8_t)atoi(mqttPayload);

		_TOTE_DEBUG_LEVEL_1_VALUE_("mqttCallback", "myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T ha cambiado a :", myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T);

		// Actualizo variables globales en la EEPROM.
		eepromUpdateData();

		// Salimos.
		return;
	}


	// Evitamos que muerda el perro.
	yield();

	// casa/DEVICE_ID/timer_duration
	if (strcmp(topic, "casa/" DEVICE_ID "/timer_duration") == 0) {
		_TOTE_DEBUG_LEVEL_1_("mqttCallback", "Detectado topic casa/DEVICE_ID/timer_duration");

		// Actualizo el valor de la duración de la temporización.
		myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T = (uint16_t)atoi(mqttPayload);

		_TOTE_DEBUG_LEVEL_1_VALUE_("mqttCallback", "myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T ha cambiado a :", myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T);

		// Actualizo variables globales en la EEPROM.
		eepromUpdateData();

		// Salimos.
		return;
	}

	// Evitamos que muerda el perro.
	yield();

	// Casa/DEVICE_ID/masterSwitch
	if (strcmp(topic, "casa/" DEVICE_ID  "/masterSwitch") == 0) {
		uint8_t cmd = (uint8_t)atoi(mqttPayload);

		_TOTE_DEBUG_LEVEL_1_VALUE_("mqttCallback", "Detectado topic casa/" DEVICE_ID "/masterSwitch con el valor: ", cmd);

		if (isRelayON && cmd == 0) {
			// Apago el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_OFF);

			// Salimos.
			return;
		}

		if (!isRelayON && cmd == 1) {
			// Enciendo el Relé
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_ON);

			// Salimos.
			return;
		}
	}
}

void mqttCheckConnectionStatusCallback(void) {
	// Si el estado es diferente a 0 (conexión correcta) fuerzo la reconexión.
	// Ver 'mqttGetState' para valores de retorno.
	if (!myMqttClient.connected()) {
		_TOTE_DEBUG_LEVEL_1_("mqttCheckConnectionStatusCallback", "Intentando reconectar con servidor MQTT.");

		// Fuerzo la reconexión.
		mqttReconnect();

		// Hasta la próxima.
		return;
	}

	// Consulto el estado de la última operación con el servidor MQTT.
	int status = myMqttClient.state();

	switch (status) {
	case -4:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECTION_TIMEOUT");
		break;

	case -3:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECTION_LOST - the network connection was broken.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECTION_LOST");
		break;

	case -2:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECT_FAILED - the network connection failed.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECT_FAILED");
		break;

	case -1:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_DISCONNECTED - the client is disconnected cleanly.");
		strcpy(mqttLastStateOpTxt, "MQTT_DISCONNECTED");
		break;

	case 0:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECTED - the client is connected.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECTED");
		break;

	case 1:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECT_BAD_PROTOCOL");
		break;

	case 2:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECT_BAD_CLIENT_ID");
		break;

	case 3:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECT_UNAVAILABLE");
		break;

	case 4:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECT_BAD_CREDENTIALS - the username / password were rejected.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECT_BAD_CREDENTIALS");
		break;

	case 5:
		_TOTE_DEBUG_LEVEL_2_("mqttGetState", "Mensaje de la ultima operacion con el servidor MQTT: MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect.");
		strcpy(mqttLastStateOpTxt, "MQTT_CONNECT_UNAUTHORIZED");
		break;
	}
}

void sendTimerState(boolean theState) {
	if (theState) {
		_TOTE_DEBUG_LEVEL_1_("sendTimerState", "Indicando que el temporizador automatico ha ENCENDIDO el rele");
		myMqttClient.publish("casa/" DEVICE_ID  "/timer_state", "SI");
	}
	else {
		_TOTE_DEBUG_LEVEL_1_("sendTimerState", "Indicando que el temporizador automatico ha APAGADO el rele");
		myMqttClient.publish("casa/" DEVICE_ID  "/timer_state", "NO");
	}
}

void sendPowerOnStatusCallback(void) {
	// Enviamos al broker el estado de encendido del Relay.
	if (isRelayON) {
		// Envío topic indicando que el equipo está encendido.
		_TOTE_DEBUG_LEVEL_1_("mqttCheckConnectionStatusCallback", "Enviando topic 'casa/" DEVICE_ID "/power_on_status_refresh' con el valor 'ENCENDIDO'");

		myMqttClient.publish("casa/" DEVICE_ID  "/power_on_status_refresh", "ENCENDIDO");
	}
	else {
		// Envío topic indicando que el equipo está apagado.
		_TOTE_DEBUG_LEVEL_1_("mqttCheckConnectionStatusCallback", "Enviando topic 'casa/" DEVICE_ID "/power_on_status_refresh' con el valor 'APAGADO'");

		myMqttClient.publish("casa/" DEVICE_ID  "/power_on_status_refresh", "APAGADO");
	}
}

void sendRelayCommand(uint8_t theCommand) {
	switch (theCommand) {
	case RELAY_COMMAND::RELAY_POWER_ON:
		isRelayON = true;
		break;

	case RELAY_COMMAND::RELAY_POWER_OFF:
		isRelayON = false;
		break;

	case RELAY_COMMAND::RELAY_SWITCH_POWER:
		isRelayON = !isRelayON;
		break;
	}

	_TOTE_DEBUG_LEVEL_1_VALUE_("sendRelayCommand", "Estableciendo el pin de excitacion de la bobina del rele a: ", isRelayON);

	// Actualizo el valor eléctrico del pin.
	digitalWrite(RELE_SGN_PIN, isRelayON);
}

void relayAutomaticOnOffTimerCallback(void) {
	// Si se dan las condiciones enciende o apaga por software el Relé

	_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T: ", myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T);

	// Si no está habilitado el temporizador no se hace nada.
	if (!myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T) {
		return;
	}

	// Si la hora recibida del topic no es correcta, tampoco se hace nada.
	if (!isValidTime) {
		_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "isValidTime: ", isValidTime);
		return;
	}

	// Compruebo si el dia se ha inicializado.
	if (timerDay == NO_VALID_DAY) {
		timerDay = day(theTime);

		_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "Inicializando el valor de dia leido de epoch : ", timerDay);
	}

	 uint16_t him = myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T * 60 + myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T; // Hora inicial en minutos.
	_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "timer_initial_hoursInMinutes (him): ", him);

	_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T: ", myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T);

	uint16_t hact = hour(theTime) * 60 + minute(theTime); // Hora actual en minutos.
	_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "Hora actual en minutos: ", hact);


	uint16_t hfm = 0;  // = almacenará el cálculo de la hora final.

	// Situación 1: el inicio y apagado están dentro del mismo día. (0 <= him <= hact <= hfm <= 1440)
	if (him + myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T <= 1440) {
		_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Detectada situacion 1: inicio y apagado en el mismo dia.");

		// Calculo la hora de apagado.
		hfm = him + myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T;
		_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "Hora final (de apagado) en minutos calculada: ", hfm);

		// Compruebo si la hora actual está dentro de la ventana de disparo.
		if (hact >= him && hact <= hfm) {
			// Enciendo el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_ON);

			_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Encendiendo rele");

			// Actualizo el nodo en el dashboard
			sendTimerState(true);
		}
		else {
			// Apago el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_OFF);

			_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Apagando rele");

			// Actualizo el nodo en el dashboard
			sendTimerState(false);
		}

		// Nada más que hacer.
		return;
	}

	// Situación 2: El inicio y apagado estan en diferentes dias. En primer lugar debo cazar el cambio de día.
	// Para ello me ayudo de una variable auxiliar que se llama 'timerDay'.
	// Si 'timerDay' coincide con day(epoch) entonces no se han pasado las 24 horas y estoy en la parte 'A' del algoritmo.
	// En caso contrario estoy en la parte 'B'

	if (timerDay == day(theTime)) { // Estamos en la parte 'A' del algoritmo (hasta 1440)
		_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Detectada situacion 2A: inicio y apagado en diferente dia, hora actual <= 1440.");

		// Se debe encender desde 'him' hasta 1440. Como el tiempo en minutos no puede ser mayor de 1440, solo solo compruebo 'him'
		if (hact >= him) {
			// Enciendo el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_ON);

			_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Encendiendo el rele");

			// Actualizo el nodo en el dashboard
			sendTimerState(true);
		}
		else {
			// Apago el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_OFF);

			_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Apagando rele");

			// Actualizo el nodo en el dashboard
			sendTimerState(false);
		}
	}
	else { // Estamos en la parte 'B'
		_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Detectada situacion 2B: inicio y apagado en diferente dia, hora actual > 1440, es decir >= 0 ");

		// En este caso, se debe encender desde el minuto cero hasta 'hfm'
		hfm = myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T - (1440 - him);
		_TOTE_DEBUG_LEVEL_1_VALUE_("relayAutomaticOnOffTimerCallback", "Hora final (de apagado) en minutos calculada: ", hfm);

		if (hact <= hfm) {
			// Enciendo el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_ON);

			_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Encendiendo rele");

			// Actualizo el nodo en el dashboard
			sendTimerState(true);
		}
		else { // Ya se ha salido por la derecha de la parte 'B'.
			timerDay = day(theTime); // Actualizo 'timerDay' para indicar que pasamos al día siguiente.

			// Apago el Relé.
			sendRelayCommand(RELAY_COMMAND::RELAY_POWER_OFF);

			_TOTE_DEBUG_LEVEL_1_("relayAutomaticOnOffTimerCallback", "Apagando el rele");

			// Actualizo el nodo en el dashboard
			sendTimerState(false);
		}
	}
}

void sendTestResponse(void) {
	// Array para construir el mensaje a enviar.
	static char mqttMsg[MAX_MQTT_CHARS];

	_TOTE_DEBUG_LEVEL_1_("sendTestResponse", "Entrando en funcion de testing. Envio mensaje con la hora actualizada.");

	if (isValidTime) {
		// Formateo la fecha.
		sprintf(mqttMsg, "Respuesta del test: %s: La fecha es %02d/%02d/%04d  %02d:%02d:%02d", mqttClientID, day(theTime), month(theTime), year(theTime), hour(theTime), minute(theTime), second(theTime));

		// Envío mensaje.
		myMqttClient.publish("casa/sonoff20190307/test_refresh", mqttMsg);
	}
	else {
		// Envío mensaje.
		myMqttClient.publish("casa/sonoff20190307/test_refresh", "La fecha y hora aun no son correctas");
	}
}

void wifiCheckSignalStrengthCallback(void) {
	// Leo el valor de la intensidad de la señal WiFi
	wifiSignal = WiFi.RSSI();

	_TOTE_DEBUG_LEVEL_2_VALUE_("wifiCheckSignalStrengthCallback", "La intensidad de la WiFI en dBm es: ", wifiSignal);
}

void refheshNoderedDashboardsCallback(void) {
	// Array para construir el mensaje a enviar.
	static char mqttMsg[MAX_MQTT_CHARS];

	_TOTE_DEBUG_LEVEL_1_("refheshNoderedDashboardsCallback", "Refrescando Dashboard de Node-Red.");


	// Actualizo 'timer_enabled_refresh'
	sprintf(mqttMsg, "%d", myEEPROMData.EEPROM_TIMER_ENABLED_UINT8_T);
	myMqttClient.publish("casa/" DEVICE_ID "/timer_enabled_refresh", mqttMsg);


	// Actualizo 'timer_initial_hours_refresh'
	sprintf(mqttMsg, "%d", myEEPROMData.EEPROM_TIMER_H_INIT_UINT8_T);
	myMqttClient.publish("casa/" DEVICE_ID "/timer_initial_hours_refresh", mqttMsg);

	// Actualizo 'timer_initial_minutes_refresh'
	sprintf(mqttMsg, "%d", myEEPROMData.EEPROM_TIMER_M_INIT_UINT8_T);
	myMqttClient.publish("casa/" DEVICE_ID "/timer_initial_minutes_refresh", mqttMsg);

	// Actualizo 'timer_duration'
	sprintf(mqttMsg, "%d", myEEPROMData.EEPROM_TIMER_DURATION_UINT16_T);
	myMqttClient.publish("casa/" DEVICE_ID "/timer_duration_refresh", mqttMsg);

	// Actualizo masterSwitch_refresh.
	sprintf(mqttMsg, "%d", isRelayON);
	myMqttClient.publish("casa/" DEVICE_ID "/masterSwitch_refresh", mqttMsg);

	// Actualizo campo "status Date" del Dashboard.
	if (isValidTime) {
		// Formateo la fecha.
		sprintf(mqttMsg, "Fecha: %02d/%02d/%04d  %02d:%02d:%02d", day(theTime), month(theTime), year(theTime), hour(theTime), minute(theTime), second(theTime));

		// Envío mensaje.
		myMqttClient.publish("casa/" DEVICE_ID "/status_date_refresh", mqttMsg);
	}
	else {
		// Envío mensaje.
		myMqttClient.publish("casa/" DEVICE_ID "/status_date_refresh", "La fecha y hora aun no son correctas");
	}

	// Actualizo campo "mqtt ID" del Dashboard.
	sprintf(mqttMsg, "Dispositivo: %s", mqttClientID);
	myMqttClient.publish("casa/" DEVICE_ID "/mqtt_ID_refresh", mqttMsg);

	// Actualizo campo "mqtt Last Op" del Dashboard
	sprintf(mqttMsg, "Last MQTT op: %s", mqttLastStateOpTxt);
	myMqttClient.publish("casa/" DEVICE_ID "/mqtt_Last_Op_refresh", mqttMsg);

	// Actualizo campo "WiFi Signal" del Dashboard
	sprintf(mqttMsg, "WiFi Signal: %ld dBm", wifiSignal);
	myMqttClient.publish("casa/" DEVICE_ID  "/wifi_signal_refresh", mqttMsg);

}




