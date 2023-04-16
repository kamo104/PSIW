#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define ROZMIAR_BUFORA 1
#define WYMAGANE_WODORY_W_WODZIE 2
#define WYMAGANE_TLENY_W_WODZIE 1

#define MIN_CZAS_PRODUKCJI 1
#define MAX_CZAS_PRODUKCJI 2


struct producent{
    int gotowych_atomow; // =0
    int atomy[ROZMIAR_BUFORA]; // 1:atom wodoru, 2: atom tlenu
    pthread_t watek;
};

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cnd_not_full = PTHREAD_COND_INITIALIZER;


static int ilosc_prod_tlenu=1;
static int ilosc_prod_wodoru=2;

// wskaźnik do wszystkich producentow
static struct producent* producenci_ptr;

void sproboj_stworzyc_wode(struct producent* ja){
    int ilosc_gotowych_wodorow = 0;
    int ilosc_gotowych_tlenow = 0;
    struct producent* producenci_z_worodem[WYMAGANE_WODORY_W_WODZIE];
    struct producent* producenci_z_tlenem[WYMAGANE_TLENY_W_WODZIE];

    // sprawdz producentow wodoru
    for(int i=ilosc_prod_tlenu;i<ilosc_prod_tlenu+ilosc_prod_wodoru;i++){
        if(ilosc_gotowych_wodorow==WYMAGANE_WODORY_W_WODZIE) break;
        if(producenci_ptr[i].gotowych_atomow==0 ) continue;
        
        // sprawdz bufor danego producenta czy ma wodory i jesli tak to zapisz to
        for(int j=0;j<producenci_ptr[i].gotowych_atomow;j++){
            producenci_z_worodem[ilosc_gotowych_wodorow] = &producenci_ptr[i];
            ilosc_gotowych_wodorow+=1;
            if(ilosc_gotowych_wodorow==WYMAGANE_WODORY_W_WODZIE) break;
        }
    }

    // sprawdz producentow tlenu
    for(int i=0;i<ilosc_prod_tlenu;i++){
        if(ilosc_gotowych_tlenow==WYMAGANE_TLENY_W_WODZIE) break;
        if(producenci_ptr[i].gotowych_atomow==0 ) continue;
        
        // sprawdz bufor danego producenta czy ma wodory i jesli tak to zapisz to
        for(int j=0;j<producenci_ptr[i].gotowych_atomow;j++){
            producenci_z_tlenem[ilosc_gotowych_tlenow] = &producenci_ptr[i];
            ilosc_gotowych_tlenow+=1;
            if(ilosc_gotowych_tlenow==WYMAGANE_TLENY_W_WODZIE) break;
        }
    }


    // zaktualizuj zuzyte wodory i tleny u producentow
    if(ilosc_gotowych_wodorow==WYMAGANE_WODORY_W_WODZIE && ilosc_gotowych_tlenow==WYMAGANE_TLENY_W_WODZIE){
        for(int i=0;i<WYMAGANE_WODORY_W_WODZIE;i++) (*producenci_z_worodem[i]).gotowych_atomow-=1;
        
        for(int i=0;i<WYMAGANE_TLENY_W_WODZIE;i++) (*producenci_z_tlenem[i]).gotowych_atomow-=1;
        
        printf("%s\n", "Wyprodukowano 1 cząsteczkę wody"); fflush(stdout);
        pthread_cond_broadcast( &cnd_not_full );
        
    }

}

void* produkuj_tlen(void* prod_ptr){
    struct producent* ja = (struct producent*)(prod_ptr);

    while(1){
        int czas_produkcji = MIN_CZAS_PRODUKCJI + rand()%(MAX_CZAS_PRODUKCJI+1);

        sleep(czas_produkcji);

        pthread_mutex_lock( &mtx );

        // wyprodukuj atom tlenu
        (*ja).atomy[(*ja).gotowych_atomow] = 1;
        (*ja).gotowych_atomow += 1;

        // sprawdz czy da sie zrobic czasteczke wody z twoich atomow
        sproboj_stworzyc_wode(ja);

        // jesli sie nie udalo to czekaj az ktos uzyje twojego atomu
        while((*ja).gotowych_atomow==ROZMIAR_BUFORA){
            printf("%s\n","Producent tlenu czeka aż ktoś użyje jego tlenu"); fflush(stdout);
            pthread_cond_wait( &cnd_not_full, &mtx );
        } 
        

        pthread_mutex_unlock( &mtx );
    }   
    

}

void* produkuj_wodor(void* prod_ptr){
    struct producent* ja = (struct producent*)(prod_ptr);

    while(1){
        int czas_produkcji = MIN_CZAS_PRODUKCJI + rand()%(MAX_CZAS_PRODUKCJI+1);

        sleep(czas_produkcji);

        pthread_mutex_lock( &mtx );

        // wyprodukuj atom tlenu
        (*ja).atomy[(*ja).gotowych_atomow] = 2;
        (*ja).gotowych_atomow += 1;

        // sprawdz czy da sie zrobic czasteczke wody z twoich atomow
        sproboj_stworzyc_wode(ja);

        // jesli sie nie udalo to czekaj az ktos uzyje twojego atomu
        while((*ja).gotowych_atomow==ROZMIAR_BUFORA){
            printf("%s\n","Producent wodoru czeka aż ktoś użyje jego wodoru"); fflush(stdout);
            pthread_cond_wait( &cnd_not_full, &mtx );
        } 
        

        pthread_mutex_unlock( &mtx );
    }   
    

}

int main(){

    // ilosc producentow obu atomow
    printf("%s\n","Podaj liczbę producentów tlenu którzy będą pracować:");
    scanf("%d", &ilosc_prod_tlenu);

    printf("%s\n","Podaj liczbę producentów wodoru którzy będą pracować:");
    scanf("%d", &ilosc_prod_wodoru);



    // inicjalizacja producentow
    struct producent producenci[ilosc_prod_tlenu+ilosc_prod_wodoru]; producenci_ptr = producenci;

    for(int i=0;i<ilosc_prod_tlenu;i++){
        producenci[i].gotowych_atomow = 0;
        pthread_create(&producenci[i].watek, NULL, produkuj_tlen, &producenci[i]);
    }
    for(int i=ilosc_prod_tlenu;i<ilosc_prod_tlenu+ilosc_prod_wodoru;i++){
        producenci[i].gotowych_atomow = 0;
        pthread_create(&producenci[i].watek, NULL, produkuj_wodor, &producenci[i]);
    }

    pthread_join(producenci[0].watek,NULL);
    

    return 0;
}