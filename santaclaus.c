#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>

#define MAX_ELVES 22
#define MAX_REINDEERS 9

static int elves=0;
static int reindeers=0;
static sem_t santaSem;
static sem_t reindeerSem;
static sem_t elfSem;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t pomocE;
static int bozic=0;

static char *irvasi[]={"Dasher","Dancer","Prancer","Vixen","Comet","Cupid","Donner","Blitzen","Rudolph"};
static int helpElves[3];

void *santa_claus(){

    while(1){
        sem_wait(&santaSem);
        pthread_mutex_lock(&mutex);
        
        if(reindeers==MAX_REINDEERS){
            printf("\n****   Santa: Pripermanje sanki   ****\n\n");
            for(int i=0;i<MAX_REINDEERS;i++)sem_post(&reindeerSem);
            reindeers-=MAX_REINDEERS;
            break;
        }
        else if(elves==3){
            printf("\n****   Santa: Pomaganje vilenjacima: %d, %d, %d  ****\n\n",helpElves[0],helpElves[1],helpElves[2]);
            
            for(int i=0;i<3;i++)sem_post(&pomocE);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_mutex_unlock(&mutex);
    sem_wait(&santaSem);//nije neophodan deo resenja uvedeno radi ispisa
    printf("\n****   Srecan bozic!  ****\n\n");

}

void *elve(void *arg){

    sleep(rand()%20);
    int br_vilenjaka=*(int*)arg;
    //printf("%d. vilenjak radi na pravljenju igracaka...\n",br_vilenjaka+1);
 
        if((rand()%12)%3==0){
            sem_wait(&elfSem);
            printf("Vilenjak: %d trazi pomoc kod sante \n",br_vilenjaka+1);
            pthread_mutex_lock(&mutex);
            elves++;
            if(elves==3)sem_post(&santaSem);
            else sem_post(&elfSem);
            pthread_mutex_unlock(&mutex);
         

            helpElves[elves-1]=br_vilenjaka+1;
            sem_wait(&pomocE);
        //obezbedjuje da se broj vilenjaka ne smanji (i onemoguci drugima da dodju do sante) pre nego sto se santa probudi i pomogne vilenjacima
        
            pthread_mutex_lock(&mutex);
            elves--;
            if(elves==0)sem_post(&elfSem);
            pthread_mutex_unlock(&mutex); 
        }
  
    free(arg);

}

void *reindeer(void *arg){

    sleep(rand()%20);
    int br_irvasa=*(int*)arg;
    printf(">>>%s se vratio sa odmora\n",irvasi[br_irvasa]);

    pthread_mutex_lock(&mutex);
    reindeers++;
    if(reindeers==MAX_REINDEERS) sem_post(&santaSem);
    pthread_mutex_unlock(&mutex);
    
    sem_wait(&reindeerSem);
    printf("%s se pricvrscuje za sanke\n",irvasi[br_irvasa]);
    
    pthread_mutex_lock(&mutex);
    bozic++;
    if(bozic==9)sem_post(&santaSem);//nije neophodan deo resenja uvedeno radi ispisa, odnosi se na linije 43-44
    pthread_mutex_unlock(&mutex);

    free(arg);

}

int main(){

    pthread_t santa;
    pthread_t elves[MAX_ELVES];
    pthread_t reindeers[MAX_REINDEERS];
    sem_init(&santaSem,0,0);
    sem_init(&reindeerSem,0,0);
    sem_init(&elfSem,0,1);
    sem_init(&pomocE,0,0);
    srand(time(NULL));
    

    pthread_create(&santa,NULL,santa_claus,NULL);
    for(int i=0;i<MAX_ELVES;i++){
        int *value=malloc(sizeof(int));
        *value=i;
        if(pthread_create(elves+i,NULL,elve,value)!=0){
            perror("Greska pri kreiranju niti\n");
            return -1;
        }
    }
    
    for(int i=0;i<MAX_REINDEERS;i++){
        int *value=malloc(sizeof(int));
        *value=i;
        if(pthread_create(reindeers+i,NULL,reindeer,value)!=0){
            perror("Greska pri kreiranju niti\n");
            return -1;
        }
    }


    if(pthread_join(santa,NULL)!=0){
        return -1;
    }

    sem_destroy(&santaSem);
    sem_destroy(&reindeerSem);
    sem_destroy(&elfSem);
    return 0;
}
