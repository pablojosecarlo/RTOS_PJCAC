# RTOS_PJCAC
Trabajo Práctico de la materia RTOS 1 

Link al video:
https://photos.google.com/photo/AF1QipO916TqWInoyGqCs_naZS0z9TwQ1eBmq8-sPQiH


El dispositivo es parte de un escáner magnético de superficies, que va relevando los valores del campo B a medida que se desplaza. Consiste en un encoder, un sensor magnético i2c y un display lcd i2c. El sensor no presentaba mayores dificultades pues tiene una interrupción que avisa cuando está disponible una nueva medición.  

Originalmente el proyecto contaba con un encoder absoluto de 12 bits , pero eran demasiados pines para andar cableando y además la alimentacion era de 12 Volts.
Se reemplazó por un encoder incremental. A los fines prácticos el cambio es mínimo pues los eventos de rotación se detectan en el pin lsb.

Se implementaron 4 interrupciones de GPIO adaptando la librería button.h del firmware_v3 para que las funciones de callback acepten argumentos. 
Igualmente hubo que implementar una tarea que poollea las MEF de las interrupciones. (je je . .nada es gratis !!)

Lo más complicado fué adaptar las funciones del lcd i2c para que funcionen con las llamadas del freeRTOS. Se perdieron incontables horas con ese tema y cuando finalmente se solucionó, quedó tan sencillo y elegante que dá vergüenza no haberlo resuelto antes.
Se usó un único queue muy cargado, tanto para los comandos, como para los strings. Originalmente había dos queues separados, pero el orden de la carga no era el orden de la recepción, así que se adoptó un único queue y se modificaron las funciones que se aplicaban al i2c.

No hizo falta implementar ninguna funcionalidad fromTheInterrupt. Las interrupciones son muy sencillas y rápidas. lo verdaderamente pesado es el display lcd 

Tampoco hizo falta mingún semaforo Mutex. Con binarios que abilitaran sucesivamente del encoder al sensor magnético y del sensor a la puesta en pantalla fue suficiente.

No se hizo a tiempo para implementar el back to back con un Wifi, tambien pensaba usar un buetooth, pero tampoco dió el tiempo.

Pablo J.C Alonso Castillo
