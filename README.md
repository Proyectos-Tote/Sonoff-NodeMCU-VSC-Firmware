# Sonoff-NodeMCU-VSC-Firmware
Interruptor WiFi basado en NodeMCU

El código tiene que ser recompilado (y cargado) para cada nuevo dispositivo. Esto se hace identificando al
dispositivo mediante una instrucción "define" en el código fuente, tal que así: 

    // Identificación del dispositivo. Para que sea único, pongo en el nombre la fecha de fabricación.
    #define DEVICE_ID "NODEMCU-20191215"

El dispositivo emite un punto de acceso para poder conectarnos desde un móvil y configurarle el acceso a
una red WiFi. El SSID se define también el el código mediante otro "define":

    #define DEVICE_SSID "TOTEESPCFG20191215"   // El SSID debe ser único, pongo la fecha de fabricación (15/12/2019) como GUID.


Cuando se inicia el dispositivo, entra en un bucle de espera para dar posibilidad de pulsar D2 (GPIO4). Este tiempo 
viene dado por el define CONFIG_WINDOW. El led parpadeará rápido durante este tiempo. Si se pulsa, emite un AP con 
el SSID dado por su correspondiente	definición en el código, así como el PASS.

ESP8266 emite un AP a cuya red debemos conectarnos y monta un servidor http para facilitar la configuración al AP
verdadero. la IP de este servidor es 192.168.4.1 y nos podemos conectar con un móvil, por ejemplo. Una vez configurado, el
dispositivo se reinicia. Debemos esperar sin hacer nada a que pase de nuevo el periodo de configuración, tras lo cual
se conecta al AP correcto y después al servidor MQTT en 192.168.1.200

Cuando el dispositivo pierde la conexión con el AP, por ejemplo, debido a que éste se ha caido o un corte eléctrico, 
el ESP8266 se reiniciará comenzando de nuevo todo el procedimiento.

El proyecto hace uso de una serie de librerías. Estas son:

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
