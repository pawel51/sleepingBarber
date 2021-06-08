#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

#define MIN_CLIENT_INTERVAL 100000
#define MAX_CLIENT_INTERVAL 400000
#define MIN_CUT_INTERVAL 600000
#define MAX_CUT_INTERVAL 800000

sem_t barber_ready, customer_ready, modifySeats, res_sem, printer_sem;

int chair_cnt = 10, taken_seats = 0, no_served_custs = 0, res_cnt = 0, debug = 0, in = 0;

struct LinkedList
{
    int data;
    struct LinkedList *next;
};

typedef struct LinkedList *node; //Define node as pointer of data type struct LinkedList

node resigned = NULL, waiting = NULL;

node createNode()
{
    node temp;                                      // declare a node
    temp = (node)malloc(sizeof(struct LinkedList)); // allocate memory using malloc()
    if (!temp)
    {
        perror("create node malloc error");
    }
    temp->next = NULL; // make next point to NULL
    return temp;       //return the new node
}

node addNode(node head, int value)
{
    node temp, p;        // declare two nodes temp and p
    temp = createNode(); //createNode will return a new node with data = value and next pointing to NULL.
    temp->data = value;  // add element's value to data part of node
    if (head == NULL)
    {
        head = temp; //when linked list is empty
    }
    else
    {
        p = head; //assign head to p
        while (p->next != NULL)
        {
            p = p->next; //traverse the list until p is the last node.The last node always points to NULL.
        }
        p->next = temp; //Point the previous last node to the new node created.
    }
    return head;
}

node popNode(node head, int value)
{
    node temp, p;
    if (value == -1 && head != NULL)
    {
        temp = head->next;
        head = NULL;
        return temp;
    }

    if (head == NULL)
    {
        return head;
    }
    else
    {
        p = head;
        if (p->data == value)
        {
            fprintf(stdout, "value poped: %d\n", p->data);
            head = p->next;

            return head;
        }
        else
        {
            while (p->data != value)
            {
                temp = p;
                p = p->next;
                if (p == NULL)
                {
                    return head;
                }
                if (p->data == value)
                {
                    fprintf(stdout, "\nvalue poped: %d", p->data);
                    temp->next = p->next;
                    return head;
                }
            }
        }
    }
}

void printList(node head)
{
    node p;
    p = head;
    printf("[");
    while (p != NULL)
    {
        fprintf(stdout, "%d, ", p->data);
        p = p->next;
    }
    printf("]\n");
}

void print_globals()
{
    if (debug)
    {
        fprintf(stdout, "Clients who resigned: ");
        printList(resigned);
        fprintf(stdout, "Clients in Waiting room: ");
        printList(waiting);
    }
    fprintf(stdout, "Res: %d    WRomm: %d/%d    [in: %d]\n\n", res_cnt, taken_seats, chair_cnt, in);
}

/*
* Funkcja wątku fryzjera, nie przyjmuje żadnych 
* zadanych parametrów
*/
void *barber_function(void *idp)
{
    while (1)
    {
        /* Zamyka semafor "customer_ready" - czekaj na klienta */
        if (sem_wait(&customer_ready) == -1)
        {
            perror("sem_wait error");
        }

        /* Zamyka semafor "modifySeats" - fryzjer próbuje wziąć klienta */
        if (sem_wait(&modifySeats) == -1)
        {
            perror("sem_wait error");
        }

        /* Fryzjer bierze klienta na fotel i zmniejsza zajęte miejsca o 1 */
        taken_seats--;
        /* Klient zajmuje fotel */
        in = waiting->data;
        /* Klient zwalnia miejsce */
        waiting = popNode(waiting, -1);
        

        /* Otwiera "barber_ready" - barber może zaczynać strzyżenie" */
        if (sem_post(&barber_ready) == -1)
        {
            perror("sem_post error");
        }
        /* Otwiera semafor "modifySeats" - klienci mogą zajmować miejsca*/
        if (sem_post(&modifySeats) == -1)
        {
            perror("sem_post error");
        }

        /* Fryzjer rozpoczyna strzyżenie które trwa od min do max ms */
        usleep(rand() % (MAX_CUT_INTERVAL - MIN_CUT_INTERVAL + 1) + MIN_CUT_INTERVAL);
    }
}

/*
* Funkcja wątku klienta, jako parametr 
* przyjmuje id wątku rosnące od 1 do inf
*/
void customer_function(void *idp)
{

    /* Zamyka semafor "modifySeats" - klient czeka na sprawdzenie liczby zejętych miejsc */
    if (sem_wait(&modifySeats) == -1)
    {
        perror("sem_wait error");
    }

    /* Jeżeli w poczekalni są jeszcze miejsca*/
    if (taken_seats < chair_cnt)
    {
        /* Zajmij miejsce */

        taken_seats++;
        waiting = addNode(waiting, *(int *)idp);

        /* Zablokuj semafor "printer_sem" - czekaj na dostępność drukarki */
        if (sem_wait(&printer_sem) == -1)
        {
            perror("sem_wait error");
        }
        print_globals();
        /* Odblokuj możliwość korzystania z drukarki */
        if (sem_post(&printer_sem) == -1)
        {
            perror("sem_post of printer_sem");
        }

        /* Odblokuj semafor "customer_ready" - klient siedzi i jest gotowy do strzyżenia */
        if (sem_post(&customer_ready) == -1)
        {
            perror("sem_post error");
        }

        /* Odblokuj semafor "modifySeats" - kolejny klient może siadać lub wstawać*/
        if (sem_post(&modifySeats) == -1)
        {
            perror("sem_post error");
        }

        /* Czekaj na semafor "barber_ready" - czekaj aż fryzjer będzie gotowy do strzyżenia */
        if (sem_wait(&barber_ready) == -1)
        {
            perror("sem_wait error");
        }

        /* Zablokuj semafor "printer_sem" - czekaj na dostępność drukarki */
        if (sem_wait(&printer_sem) == -1)
        {
            perror("sem_wait of printer_sem");
        }
        print_globals();
        /* Odblokuj możliwość korzystania z drukarki */
        if (sem_post(&printer_sem) == -1)
        {
            perror("sem_post error");
        }
    }
    else
    {
        /* Odblokuj semafor "modifySeats" zakończ próbę wejścia, strzyż się kiedy indziej  */
        sem_post(&modifySeats);
        /* Zablokuj semafor "res_sem" czekaj na wyjście z lokalu*/
        if (sem_wait(&res_sem) == -1)
        {
            perror("sem_wait error");
        }
        res_cnt++;
        /* Dodanie do listy klientów którzy zrezygnowali z usługi golibrody z powodu braku miejsc*/
        resigned = addNode(resigned, *(int *)idp);
        /* DEBUG Update listy zrezygnowanych oraz ich liczby*/

        /* Zablokuj semafor "printer_sem" - czekaj na dostępność drukarki */
        if (sem_wait(&printer_sem) == -1)
        {
            perror("sem_wait error");
        }
        print_globals();
        /* Odblokuj możliwość korzystania z drukarki */
        if (sem_post(&printer_sem) == -1)
        {
            perror("sem_post error");
        }

        /* Odblokuj semafor "res_sem" - klient opuszcza lokal */
        if (sem_post(&res_sem) == -1)
        {
            perror("sem_post error");
        }
    }
    /* Klient opuszcza naszą rzeczywistość */
    pthread_exit(NULL);
}

void *make_customer_function()
{
    int tmp;
    int i = 0;
    while (1)
    {
        i += 1;
        /* Stwórz wątek nowego klienta */
        pthread_t customer_thread;
        tmp = pthread_create(&customer_thread, NULL, (void *)customer_function, &i);
        if (tmp)
            fprintf(stderr, "Failed to create customer thread id: %d;\n ERROR: %s\n", i, strerror(tmp));

        /* Czekaj min-max ms przed utworzeniem nowego klienta */
        usleep(rand() % (MAX_CLIENT_INTERVAL - MIN_CLIENT_INTERVAL + 1) + MIN_CLIENT_INTERVAL);
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        char *debug_string = argv[1];
        if (!strcmp(debug_string, "debug"))
            debug = 1;
    }

    srand(time(NULL));

    int tmp;

    pthread_t barber1;

    pthread_t customer_maker;

    /* Inicjalizacja semaforów */
    if (sem_init(&customer_ready, 0, 0))
    {
        perror("sem_init customer_ready error");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&barber_ready, 0, 0))
    {
        perror("sem_init barber_ready error");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&modifySeats, 0, 1))
    {
        perror("sem_init modifySeats error");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&printer_sem, 0, 1))
    {
        perror("sem_init printer_sem error");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&res_sem, 0, 1))
    {
        perror("sem_init res_sem error");
        exit(EXIT_FAILURE);
    }

    /* Stwórz wątek barbera*/
    tmp = pthread_create(&barber1, NULL, (void *)barber_function, NULL);
    if (tmp)
        fprintf(stderr, "Failed to create barber thread;\n ERROR: %s\n", strerror(tmp));
    /* Stwórz wątek generatora klientów*/
    tmp = pthread_create(&customer_maker, NULL, (void *)make_customer_function, NULL);
    if (tmp)
        fprintf(stderr, "Failed to create ClientMaker thread;\n ERROR: %s\n", strerror(tmp));

    /* Synchronizacja wątków */
    pthread_join(barber1, NULL);
    pthread_join(customer_maker, NULL);
}