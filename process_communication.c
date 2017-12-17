//Including system libraries
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include<stdbool.h>
#include <errno.h>

//Including user libraries
#include "process_communication.h"


/**********************************************************************************
*
**********************************************************************************/
unsigned short get_member_count(int sid){
        union semun semopts;
        struct semid_ds mysemds;

        semopts.buf = &mysemds;

        /* Return number of members in the semaphore set */
        return(semopts.buf->sem_nsems);
}


/**********************************************************************************
*Locks semaphore 'member' with in sempahore set with id "sid"
**********************************************************************************/
void locksem(int sid, int member){

        struct sembuf sem_lock={0, -1, 0}; //IPC_NO_WAIT removed

        /* Attempt to lock the semaphore set */

        sem_lock.sem_num = member;
        
        if((semop(sid, &sem_lock, 1)) == -1){
                int errnum = errno;
                fprintf(stderr, "Lock failed (sem %d of set %d)\n",member,sid);
                fprintf(stderr, "Value of errno: %d\n",errno);
                fprintf(stderr, "Error: %s \n",strerror(errnum));
                exit(1);
        }
        else
            printf("Semaphore(sem %d of set %d) - 1 , value : %d\n",member,sid,getval(sid,member));


}


/**********************************************************************************
*
**********************************************************************************/
void unlocksem(int sid, int member){
        struct sembuf sem_unlock={member, 1,0}; //IPC_NO_WAIT removed

        sem_unlock.sem_num = member;

        /* Attempt to lock the semaphore set */
        if((semop(sid, &sem_unlock, 1)) == -1){
                int errnum = errno;
                fprintf(stderr, "Unlock failed (sem %d of set %d)\n",member,sid);
                fprintf(stderr, "Value of errno: %d)\n",errno);
                fprintf(stderr, "Error: %s \n",strerror(errnum));
                exit(1);
        }
        else
            printf("Semaphore(sem %d of set %d) + 1 , value : %d\n",member,sid,getval(sid,member));

}


/**********************************************************************************
*
**********************************************************************************/
void createsem(int *sid, key_t key, int members){


    if((*sid = semget(key, members, IPC_CREAT|IPC_EXCL|0666)) == -1) {
        fprintf(stderr, "Semaphore set already exists!\n");
        exit(1);
    }
    printf("Created new semaphore %d set with %d members\n ",*sid,members);

}

void createqueue(int *msgqueue_id, key_t key_q, int members){

    /* Open the queue - create if necessary */
    if((*msgqueue_id = msgget(key_q, IPC_CREAT|0660)) == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Created new message queue %d set with %d members\n ",*msgqueue_id,members);

}
        
/**********************************************************************************
*
**********************************************************************************/
int getval(int sid, int member){
        int semval;

        semval = semctl(sid, member, GETVAL, 0);
        return(semval);
}


/**********************************************************************************
*
**********************************************************************************/
void setval( int sid, int semnum, int value){
        union semun semopts;    

        semopts.val = value;
        semctl(sid, semnum, SETVAL, semopts);
}



/**********************************************************************************
*
**********************************************************************************/
void setall(int sid,int value){
    union semun semopts;
    semopts.val = value;

    int members = get_member_count(sid);
    for(int cntr=0; cntr<members; cntr++){
        semctl(sid,cntr, SETVAL, semopts);    

    }   
}


/**********************************************************************************
*
**********************************************************************************/
void writeshm(point* segptr,int index, point value){
    segptr[index] = value;
}


/**********************************************************************************
*
**********************************************************************************/
point readshm(point* segptr, int index){
    return segptr[index];
}
/**********************************************************************************
*
**********************************************************************************/

void send_message(int qid, struct mymsgbuf *qbuf){
    /* Send a message to the queue */
   // printf("Sending a message ...\n");
    int length = sizeof(struct mymsgbuf) - sizeof(long);
    if((msgsnd(qid, (struct mymsgbuf *)qbuf,length, 0)) ==-1){
        perror("msgsnd");
        exit(1);
    }
}
/**********************************************************************************
*
**********************************************************************************/
void read_message(int qid, struct mymsgbuf *qbuf, long type){
    /* Read a message from the queue */
   // printf("Reading a message ...\n");
    int length = sizeof(struct mymsgbuf) - sizeof(long);
    
   if((msgrcv(qid, (struct mymsgbuf *)qbuf, length, type, 0)) ==-1){
        perror("msgrcv");
        exit(1);
    } 
       // printf("Type: %ld Text: %s\n", qbuf->mtype, qbuf->mtext);
}


/**********************************************************************************
*
**********************************************************************************/
void remove_sem(int semid)
{
        semctl(semid, 0, IPC_RMID, 0);
        printf("Semaphore set marked for deletion.\n");
}


/**********************************************************************************
*
**********************************************************************************/
void remove_shm(int shmid)
{
        shmctl(shmid, IPC_RMID, 0);
        printf("Shared memory segment marked for deletion.\n");
}


/**********************************************************************************
*
**********************************************************************************/
void remove_queue(int qid)
{
        /* Remove the queue */
        msgctl(qid, IPC_RMID, 0);
        printf("Message queue marked for deletion.\n");
}