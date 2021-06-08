#sleeping barber
Propozycja rozwiązania problemu śpiącego fryzjera wykorzystując semafory.

Zawartość projektu:
    - src/sleepingBarber.c      
    - src/Makefile              
    - docs/sprawozdanie         
    - READ.me 

Instrukcja: 
1. Kompilacja programu za pomocą polecenia: make ./sb_sem
2. Dwie opcje uruchomienia:
    2.1 Bez parametrów: ./sb_sem
    
        Res: 0    WRomm: 1/10    [in: 0]
        
        Res: 0    WRomm: 0/10    [in: 1]
        
        Res: 0    WRomm: 1/10    [in: 1]
        
        Res - liczba osób, które zrezygnowały
        WRomm - liczba osób w poczekalni
        in - indeks osoby na fotelu
    2.2 Debug: ./sb_sem debug
    
        Clients who resigned: [16, 18, 19, 21, 23, 24, ]  (max==inf)
        Clients in Waiting room: [9, 10, 11, 12, 13, 14, 15, 17, 20, 22, ]  (max==10)
        Res: 6    WRomm: 10/10    [in: 8]
