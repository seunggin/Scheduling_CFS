//
//  main.c
//  ku_cfs
//
//  Created by 유승진 on 2018. 3. 27..
//  Copyright © 2018년 유승진. All rights reserved.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>



float root(float A, int n) {
    const int K = 6;
    float x[6] = {1};
    for (int k = 0; k < K - 1; k++)
        x[k + 1] = (1.0 / n) * ((n - 1) * x[k] + A / pow(x[k], n - 1));
    return x[K-1];
}
// 루트의 개념을 이용도입하고 그것을 통해서 지수 계산에 이용한다.
float pow(float base, float ex){
    if (base == 0)
        return 0;
    // power of 0
    if (ex == 0){
        return 1;
        // negative exponenet
    }else if( ex < 0){
        return 1 / pow(base, -ex);
        // fractional exponent
    }else if (ex > 0 && ex < 1){
        return root(base, 1/ex);
    }else if ((int)ex % 2 == 0){
        float half_pow = pow(base, ex/2);
        return half_pow * half_pow;
        //integer exponenet
    }else{
        return base * pow(base, ex - 1);
    }
}
// 지수 계산에 이용한다.
typedef struct process {
    pid_t p_id;
    //프로세스 아이디
    int nice;
    //프로세스의 나이스 값
    char work;
    //프로세스가 ku_app 에 전달해주는 변수
    float vruntime ;
    //프로세스 가 돌아간 시간
    struct process *next;
}PROCESS;
// 각 프로세스의 id 와 nice 그리고 runtime을 저장 해준다.


PROCESS *Head;
PROCESS *Current;

void InitProcessList(){

	Head = (PROCESS *)malloc(sizeof(PROCESS));
	Head->next =  NULL;
	Current = Head;
}
void addProcess(){
	PROCESS *new = (PROCESS *)malloc(sizeof(PROCESS));
	Current->next = new;
	new->next  = NULL;
	Current = new ;
}
void releaseProcess(){
    PROCESS *buffer = Head;
    PROCESS *temp ;
    Current = buffer ;
    while( Current !=NULL){
        temp = Current->next ;
        free(Current);
        Current = temp;
    }
    
}
void printAllProcessWorkNNice(){
    PROCESS *temp ;
    temp  = Current;
    Current = Head->next;
    while(Current !=NULL){
        printf("Process %c %d\n",Current->work,Current->nice);
        Current = Current->next;
    }
    Current = temp;
    
}

void printAllProcessVruntime(){
    PROCESS *temp ;
    temp = Current ;
    Current = Head->next ;
    while( Current !=  NULL ){
        printf("Process %f\n",Current->vruntime);
        Current =  Current->next;
    }
    Current =  temp ;
    
}
void resetVruntime(){
    PROCESS *temp ;
    temp = Current;
    Current = Head;
    while(Current != NULL){
        Current->vruntime = 0;
        Current = Current->next ;
    }
    Current  = temp ;
}


void my_timer_callback( unsigned long data )
{
    printf("돌려야 할 프로그램을 선정하고 있습니다....\n");
}

//struct process pids[26];
// A~Z 까지 들어 올수 있으니 최대 26개의 알파벳이 가능하다.

int nextProcesssNum = 0 ;
int checkProcess = 0;
//프로세스가 몇개 만들어주어야 하는지 확인한다.

// 타이머 시간이 끝나면 실행되는 함수.

int main(int argc, const char * argv[]) {

	InitProcessList();
    resetVruntime();
    float  minimum[checkProcess];
    
    int number = 0 ;
    // 스케줄링에서 쓰이는 함수
    int runProcess  = 0 ;
    int arrayCheck = 0;
    
    int OSPID  = getpid();
    //OS 의 pid 를 저장합니다.
    int nice[5] = { -2 ,-1 ,0, 1, 2 };
    int niceArray[26] ;
    int niceAlloc = 0 ;
    int timeSlice = atoi(argv[6]);
    
    clock_t begin_time , end_time ;
    // 변수들 모음
//    printf("The OS PID : %d\n",getpid());
//    스케줄링 해주는 프로세스의 아이디를 적는다.
    
    if( argc != 7 || checkProcess > 26 ){
        printf("\n\n6 개의 변수를 입력 하시고 그 n1~n5 합은\n26 을 넘어서는 안된다. \n");
        printf("./ku_cfs n1 n2 n3 n4 n5 ts\n") ;
        printf("----------------예제---------------\n");
        printf("|./ku_cfs 0 1 2 0 0 30            |\n");
        printf("----------------------------------\n\n");
        exit(0);
    }
    //변수 입력값이 다를때 오류 처리를 위한 예외 처리.
    char program[26] = { 'A' ,'B' ,'C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z' };
    
    int sumOfArg = 0;
    
    for(int i =  1 ; i < 6 ; i++){
        sumOfArg += atoi(argv[i]);
    }
    printf("sumOfArg = %d \n",sumOfArg);
    for(int i =  1 ; i< 6 ; i++){
        for( int j = 0 ; j < atoi(argv[i]); j ++)
        {
            niceArray[arrayCheck] =  nice [niceAlloc];
            arrayCheck++;
        }
        niceAlloc++;
    }
    arrayCheck =0;
    
    for(int i = 0 ; i < sumOfArg; i++){
        addProcess();
        Current->p_id = fork();
        if(Current->p_id < 0){
            printf("fork() failed\n");
            exit(0);
        } else if(Current->p_id == 0 ){
            Current->p_id = getpid();
            Current->work = program[runProcess];
            Current->nice = niceArray[runProcess];
            break;
        }
        Current->work = program[runProcess];
        Current->nice = niceArray[runProcess];
        runProcess++;
        arrayCheck++;
    }
    if (OSPID == getpid()){
        printAllProcessWorkNNice();
    }
    if( OSPID != getpid()){
        char *arg[]  = { "./ku_app", &Current->work , (char *)0};
        int i  =  execvp("./ku_app",arg);
        
        if( i < 0){
            printf("execvp failed\n");
            exit(0);
        }
    }
    struct itimerval set_time_val , get_time_val ;
    sigset(SIGALRM , my_timer_callback); //
    
    set_time_val.it_value.tv_sec =  timeSlice ;
    set_time_val.it_value.tv_usec = 0 ;
    //초기 설정값
    set_time_val.it_interval.tv_sec =  timeSlice ;
    set_time_val.it_interval.tv_usec = 0 ;
    //후에 간격 설정값.
    
    //real time 측정을 위한 설정
    
    
        //vruntime 계산을 위한 초기화값.
    if(setitimer(ITIMER_REAL, &set_time_val, NULL) == -1)
    {
            perror("Failed to set virtual timer!\n");
            return  0 ;
    }
	//timer error
    
    while(1){
        
        begin_time = clock();
        kill(Current->p_id,SIGCONT);
        pause();
        kill(Current->p_id,SIGSTOP);
        end_time = clock();
        
        float min = Head->vruntime;
        
        Current->vruntime = Current->vruntime + 1.25*pow(difftime(end_time,begin_time),Current->nice);
//
//        for (; Current!=NULL;){
//            if(min >= Current->vruntime ){
//                min = Current->vruntime;
//            }
//            Current  = Current->next;
//        }
        Current  = Current->next;
        
        if(Current == NULL){
            Current = Head->next;
        }
        
        
        //printAllProcessVruntime();
    }
    releaseProcess();
    
}

// 2018.8.9 Thursday edited by Seung-Jin Yu

// 2018.3.29.Thursday designed by Seung-Jin Yu

// 2018.4.3.Tuesday updated by Seung-Jin Yu
