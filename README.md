# Part2Part

## How to run

In order to build and run the project, go to the src folder then run the command `make all`. This will compile all the necessary files. To run the server type `./server <ip> <port>` and to run the client type `./client <ip> <port>`. The ip can be your router ip which can be found by running `ifconfig | grep "inet "`, then you can choose an unsed port.

## Task:

Sa se conceapa o aplicatie de tipul peer-to-peer pentru partajarea fisierelor. Programele vor folosi socket-uri pentru comunicatie, putind fi rulate pe masini diferite astfel: programul client poate fi rulat de catre orice utilizator, iar programul server poate fi rulat doar de autor, care este si responsabil cu pornirea/oprirea serviciului. Programul server trebuie sa fie capabil sa serveasca simultan mai multe cereri de transfer provenite de la "colegii" sai din retea. Fisierele dorite vor putea fi cautate - eventual folosind expresii regulate - conform unor criterii (nume, tip, lungime etc.) in cadrul "retelei".

Resurse suplimentare: http://www.rogerclarke.com/EC/P2POview.html

```Deadline: 14.01.2021```
