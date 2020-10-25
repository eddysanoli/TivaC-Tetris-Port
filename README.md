# Port de Tetris en Tiva-C

## Descripción

Port casi completo del juego Tetris para el "Nintendo Entertainment System" programado utilizando C++ en la IDE de "Energia" que viene en conjunto con la plataforma de prototipado Tiva-C Launchpad. El port es una copia casi idéntica, con la capacidad de poder jugar al modo A (Tetris clásico) y modo B (se generan bloques aleatorios en la pantalla y el jugador debe limpiarlos) del juego. Si el jugador es capaz de limpiar cierto número de filas, este sube de nivel, su punteo incrementa y el esquema de colores de los bloques cambia. Si este alcanza un punteo lo suficientemente alto, el juego lo reconocerá y le permitirá guardarlo como un high-score haciendo uso de la tarjeta SD conectada a la Tiva-C. 

Se escribieron múltiples funciones para facilitar la creación de nuevos modos de juego y menús, incluyendo: Una función que permite modificar más fácilmente los sprites utilizados por el juego, una rutina que crea y rota los diferentes Tetrominos y finalmente una que permite utilizar la misma font que en el juego original, utilizando únicamente un string como referencia.

Claramente, el port no es perfecto. La diferencia principal con el juego original es que este no cuenta con música. El usuario es capaz de elegir la opción en el menú para cambiar de canción, pero esta no hace nada. Se logró implementar la música del "Theme A" conectando otra Tiva-C con un buzzer, pero los efectos de sonido nunca fueron implementados de ninguna forma. Otras funcionalidades no incluidas fueron algunas sprites para los fondos de los menús y la pantalla de juego, y el modo de juego de 2 jugadores.

## Hardware

- Cables Jumper
- [2 Tiva-C Launchpad](https://www.electronicasmd.com/productos/microcontroladores/launchpad/)
- [TFT LCD para Arduino Uno R3](https://www.electronicadiy.com/products/pantalla-touch-tft-2-4?_pos=5&_sid=bf7ba1fc8&_ss=r)
- [Joystick de 5 Pines](https://www.electronicadiy.com/products/joystick-5-pines?_pos=1&_sid=6bc401e56&_ss=r)
- Memoria SD de 1 GB

## Software

- [Energia IDE](https://energia.nu)

## Demostración

Hacer click en la imagen a continuación para observar un breve demo del juego en ejecución. 

[![Demo](https://img.youtube.com/vi/KmeQmH8CL3A/0.jpg)](https://www.youtube.com/watch?v=KmeQmH8CL3A "Demo")
