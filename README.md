# RTOS_PJCAC
Trabajo Práctico de la materia RTOS 1 


Originalmente el proyecto contaba con un encoder absoluto de 12 bits , pero eran demasiados pines para andar cableando y adem{as la alimentacion era de 12 Volts.
Se reemplazó por un encoder incremental. A los fines prácticos el cambio es mínimo pues los eventos de rotación se detectan en el pin lsb.

Se implementaron 4 interrupciones de GPIO adaptando la librería button.h del firmware_v3 para que las funciones de callback acepten argumentos. 
Igualmente hubo que implmentar una tarea que recorre las MEF de las interrupciones. 

Lo más complicado fué adaptar las funciones del lcd i2c para que funcionen con las llamadas del freeRTOS. Se perdieron incontables horas con ese tema y cuando finalmente se solucionó, quedó tan sencillo y elegante que dá vergüenza no haberlo resuelto antes.
Se usó un único queue muy cargado, tanto para los comandos, como para los strings. Originalmente habia dos queues separados, pero el orden de la carga no era el orden de la recepción, así que se adoptó un único queue y se modificaron las funciones que se aplicaban al i2c.

No hizo falta implementar ninguna funcionalidad fromTheInterupt. Las interrupciones son muy sencillas y rápidas. lo verdaderamente pesado es el display lcd 

Tampoco hizo falta mingún semaforo Mutex. Con binarios que abilitaran sucesivamente del encoder al sensor magnético y del sensor a la puesta en pantalla fue suficiente.

No se hizo a tiempo para implementar el back to back con un Wifi, pensabamos usar un buetooth , tampoco dió el tiempo.

Pablo J.C Alonso Castillo
