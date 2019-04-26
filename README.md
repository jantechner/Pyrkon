Kompilacja:

- `make` - kopilacja i utworzenie pliku wykonywalnego
- `make run` - uruchomienie programu 
- `make clear` - usunięcie poprzednich plików wykonywalnych 
- `make reset` - usunięcie poprzednich plików wykonywalnych i ponowna kompilacja (połączenie make clear i make)


`reset` - zresetowanie kolorów terminala i przywrócenie standardowych ustawień

Modyfikowanie typu pakietu:

1. w main.h szukamy struct packet_t i dodajemy pole np. int nowe_pole; zwiększamy FIELDNO

2. w init.c szukamy funkcji initialize i tam zamieniamy dodajemy typ pola do MPI_Datatype typy[3] = {MPI_INT, MPI_INT, MPI_INT), wyliczamy offset dla nowego pola

3. make clear; make
