# so1
University assignment for the Operating Systems I course (Software Engineering). The following requirements (in Italian) must be fulfilled by a C program using UNIX system calls.


Specifica della tesina:


Bacheca Elettronica Remota

Realizzazione di una bacheca elettronica residente su un server. Una
bacheca elettronica e' un servizio che permette ad ogni utente autorizzato
(su qualunque macchina) di inviare messaggi che possono essere letti da
ogni altro utente interessato a consultare la bacheca stessa. In questo
caso la bacheca e' costituita da un programma server che accetta e
processa sequenzialmente le richieste di uno o piu' processi client
(residenti, in generale, su macchine diverse dal server). Un client deve
fornire ad un utente le seguenti funzioni:
1.Leggere tutti i messaggi sulla bacheca elettronica.
2.Spedire un nuovo messaggio sulla bacheca elettronica.
3.Rimuovere un messaggio dalla bacheca elettronica (se appartenente
allo stesso utente che lo vuole cancellare, verifica da effettuare tramite un meccanismo di autenticazione a scelta dello studente).

Un messaggio deve contenere almeno i campi Mittente, Oggetto e Testo. Si
precisa che lo studente e' tenuto a realizzare sia il client che il
server.
