#include<pthread.h>
#include<iostream>
#include<fstream>
#include<math.h>
#include<chrono>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<string>
#include<unistd.h>
#include<vector>
#include<pthread.h>
#include<thread>
#include<random>
#include<sstream>
#include<algorithm>

#define PORT_NO 5347



std::string vectortostring(std::vector<int> vectortime){
    std::stringstream st;
    for(int i=0;i<vectortime.size();i++){
        st<<vectortime[i];
        st<<" ";
    }
    return st.str();
}

std::vector<int> stringtovector (std::string str){
    std::stringstream s(str);
    std::vector<int> vec;
    int num;
    char c;
    while(s>>num){
        vec.push_back(num);
    }
    return vec;
}

std::vector<int> NDratio(double alpha){          // Converts a double into a fraction of form p/q
    int decimalplaces = 1000;
    int GCD = std::__gcd(int(alpha*decimalplaces),decimalplaces);
    int numerator=int(alpha*decimalplaces)/GCD;
    int denominator=decimalplaces/GCD;

    std::vector<int> ret(2,0);
    ret[0]=numerator;
    ret[1]=denominator;

    return ret;
}



class argument_class_creator{
public:

    int n;
    int* socketarray;
};

class argument_class_node{
public:

    int j;
    int n;
    int* sockarr1;
    int* sockarr2;
    double lambda;
    double alpha;
    int m;
    FILE* fptr;
    int* ptr;
    std::vector<std::vector<int>> adj_list;
};





/*---------------Server thread starts its life here---------------*/

void* creator_func1(void* arg)
{

    int n = (static_cast<argument_class_creator*>(arg))->n;
    int* sockarr = (static_cast<argument_class_creator*>(arg))->socketarray;

    int main_socketid = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(main_socketid==-1)
    {
        std::cerr<<"Could not create the socket"<<std::endl;
        return nullptr;
    }

    struct sockaddr_in main_portaddr,clientportaddr;
    main_portaddr.sin_family=AF_INET;
    main_portaddr.sin_port = htons(PORT_NO);
    main_portaddr.sin_addr.s_addr = htonl(INADDR_ANY);          //Accepting on all the IP interfaces of the machine

    int status = bind(main_socketid, (struct sockaddr*)&main_portaddr, sizeof(main_portaddr));
    if (status==-1)
        {
            std::cerr<<"Could not bind the socket"<<std::endl;
            return nullptr;
        }

    status = listen(main_socketid, 3);
    if (status==-1)
        {
            std::cerr<<"Could not start listening"<<std::endl;
            return nullptr;
        }


    for(int i=0;i<n*n;i++){
        struct sockaddr_in clientportaddr;
        socklen_t clntLen = sizeof(clientportaddr);

        std::cout<<"server running"<<std::endl;

        sockarr[i]=accept(main_socketid,(struct sockaddr *)&clientportaddr,&clntLen);
        if (sockarr[i] == -1)
            {
                std::cerr<<"Could not accept the request"<<std::endl;
                return nullptr;
            }
    }

    return nullptr;
}




/*---------------Client thread starts its life here---------------*/

void* creator_func2(void* arg)
{

    int n = (static_cast<argument_class_creator*>(arg))->n;
    int* sockarr = (static_cast<argument_class_creator*>(arg))->socketarray;


    std::string IPaddress="127.0.0.1";
    int clientsockid;

    for(int i=0;i<n*n;i++){

        clientsockid = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

        sockarr[i]=clientsockid;

        if(clientsockid==-1)
        {
            std::cerr<<"Could not create the server"<<std::endl;
            return nullptr;
        }

        struct sockaddr_in portaddr;
        portaddr.sin_family=AF_INET;
        portaddr.sin_port = htons(PORT_NO);

        inet_pton(AF_INET,IPaddress.c_str(),&portaddr.sin_addr.s_addr);

        int status=connect(clientsockid, (struct sockaddr *) &portaddr,sizeof(portaddr));
        if(status==-1)
        {
            std::cerr<<"Could not connect to the server"<<std::endl;
            return nullptr;
        }
    }
    return nullptr;
}



/*---------------n node threads start their life here---------------*/

void* node_func(void* arg)
{
    int j = (static_cast<argument_class_node*>(arg))->j;
    int n = (static_cast<argument_class_node*>(arg))->n;
    int* sockarr1 = (static_cast<argument_class_node*>(arg))->sockarr1;
    int* sockarr2 = (static_cast<argument_class_node*>(arg))->sockarr2;
    double lambda = (static_cast<argument_class_node*>(arg))->lambda;
    double alpha = (static_cast<argument_class_node*>(arg))->alpha;
    int m = (static_cast<argument_class_node*>(arg))->m;
    FILE* fptr = (static_cast<argument_class_node*>(arg))->fptr;
    int* datasent = (static_cast<argument_class_node*>(arg))->ptr;
    std::vector<std::vector<int>> adjacencylist=(static_cast<argument_class_node*>(arg))->adj_list;

    datasent[j]=0;                                              // datasent[j] = number of bytes sent by node j till now

    int numerator=NDratio(alpha)[0];
    int denominator=NDratio(alpha)[1];


    std::vector<int> myvectortime(n,0);                          // Vector Clock
    std::vector<int> receivedvector(n,0);
    std::string sendvectorstring(n*11,0);
    char buffer[n*11];                                           // Buffer to receive update vector


    std::default_random_engine generator1;
    std::exponential_distribution<double> distribution1(1/lambda);      //In the assignment Lambdas are mentioned to be have units of time (in sec)
    double T1,T2;
    int t1,t2;



    fd_set currentsockets, readysockets;                    // Creating fd sets for select() function
    int maxfd=0;
    FD_ZERO (&currentsockets);
    for(int k=0;k<n;k++){
        FD_SET(sockarr2[k*n+j],&currentsockets);
        maxfd=maxfd>sockarr2[k*n+j]?maxfd:sockarr2[k*n+j];
    }
    struct timeval tv;                                      // Setting waiting time for select() function
    tv.tv_sec=0;
    tv.tv_usec = 1;



    int l=0;                        // l = no of messages sent.
    int p=1;                        // p = process id to which message will be sent

    unsigned long long int reqtime;

    while(l<m){

        for(int t=0;t<numerator;t++){                   // INTERNAL EVENTS
            myvectortime[j]+=1;
            reqtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            reqtime = ( reqtime + 19800000)%86400000;                   //A day consists of 86400000 milliseconds. 19800000 added to adjust international time delay.

            sendvectorstring=vectortostring(myvectortime);
            printf("Process %d executes internal event e%d%d at %llu:%llu:%llu:%llu, vc: [%s]\n", j+1, j+1,myvectortime[j],
                   reqtime/3600000,reqtime%3600000/60000,reqtime%60000/1000,reqtime%1000,sendvectorstring.c_str());
            fprintf(fptr,"Process %d executes internal event e%d%d at %llu:%llu:%llu:%llu, vc: [%s]\n", j+1, j+1,myvectortime[j],
                   reqtime/3600000,reqtime%3600000/60000,reqtime%60000/1000,reqtime%1000,sendvectorstring.c_str());
            T1= distribution1(generator1);
            t1 = (int)1000.0*T1;
            std::this_thread::sleep_for(std::chrono::microseconds(t1));         //Sleep for t1 microseconds
        }



        for(int q=0;q<denominator;q++){                 // SEND EVENTS

            p=(rand()%(adjacencylist[j].size()-1))+1;   // Randomly choose p

            myvectortime[j]+=1;
            sendvectorstring=vectortostring(myvectortime);

            int status = send(sockarr1[j*n+adjacencylist[j][p]-1], sendvectorstring.c_str(), sendvectorstring.size(), 0);
            if(status!=sendvectorstring.size()){
                std::cerr<<"Could not send the complete message"<<std::endl;
            }
            else {
                reqtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                reqtime = ( reqtime + 19800000)%86400000;                   //A day consists of 86400000 milliseconds. 19800000 added to adjust international time delay.
                printf("Process %d sends message m%d%d to process %d at %llu:%llu:%llu:%llu, vc: [%s]\n", j+1, j+1,myvectortime[j],adjacencylist[j][p],
                       reqtime/3600000,reqtime%3600000/60000,reqtime%60000/1000,reqtime%1000,sendvectorstring.c_str());
                fprintf(fptr,"Process %d sends message m%d%d to process %d at %llu:%llu:%llu:%llu, vc: [%s]\n", j+1, j+1,myvectortime[j],adjacencylist[j][p],
                       reqtime/3600000,reqtime%3600000/60000,reqtime%60000/1000,reqtime%1000,sendvectorstring.c_str());

                datasent[j]=datasent[j]+sendvectorstring.size();
            }

            T2= distribution1(generator1);
            t2 = (int)1000.0*T2;
            std::this_thread::sleep_for(std::chrono::microseconds(t2));         //Sleep for t2 microseconds

            l++;
        }


        readysockets=currentsockets;
        select(maxfd+1,&readysockets,NULL,NULL,&tv);

        for(int i=1;i<adjacencylist[j].size();i++){         // RECEIVE EVENTS
            if(FD_ISSET(sockarr2[(adjacencylist[j][i]-1)*n+j],&readysockets)){
                memset(buffer, 0, n*11);
                recv(sockarr2[(adjacencylist[j][i]-1)*n+j], buffer, n*11, 0);
                receivedvector=stringtovector(buffer);
                myvectortime[j]+=1;
                for(int k=0;k<n;k++){                   // Compare vector clock with received vector and update
                    myvectortime[k]=myvectortime[k]>receivedvector[k]?myvectortime[k]:receivedvector[k];
                }
                reqtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                reqtime = ( reqtime + 19800000)%86400000;                   //A day consists of 86400000 milliseconds. 19800000 added to adjust international time delay.
                printf("Process %d receives message m%d%d from process %d at %llu:%llu:%llu:%llu, vc: [%s]\n", j+1, adjacencylist[j][i],receivedvector[adjacencylist[j][i]-1],adjacencylist[j][i],
                       reqtime/3600000,reqtime%3600000/60000,reqtime%60000/1000,reqtime%1000,vectortostring(myvectortime).c_str());
                fprintf(fptr,"Process %d receives message m%d%d from process %d at %llu:%llu:%llu:%llu, vc: [%s]\n", j+1, adjacencylist[j][i],receivedvector[adjacencylist[j][i]-1],adjacencylist[j][i],
                       reqtime/3600000,reqtime%3600000/60000,reqtime%60000/1000,reqtime%1000,vectortostring(myvectortime).c_str());

            }
        }
    }

    return nullptr;

}




/*---------------Parent Function------------- */

int main()
{

  /* --------READING PARAMETERS---------*/

    std::ifstream file("inp-params.txt");
    int n;
    file>> n;                               // n = number of node thread to be used
    std::cout<<n<<std::endl;
    double lambda;
    file>> lambda;                          // lambda = average inter-event time in millisec
    std::cout<<lambda<<std::endl;
    double alpha;
    file>> alpha;                           // alpha = ratio of internal events to send events
    std::cout<<alpha<<std::endl;
    int m;
    file>> m;                               // m = number of message to be sent per thread/node.
    std::cout<<m<<std::endl;

    std::string s;

    getline(file,s);

    std::vector<std::vector<int>> adjacencylist;  // Extracting adjacency list from inp-params.txt
    
    int i=0;
    while(i<n){
        int nodeid;
        std::vector<int> v;
        getline(file,s);
        std::stringstream stream(s);
        while(stream>>nodeid){
            v.push_back(nodeid);
        }
        adjacencylist.push_back(v);
        i++;
    }
    file.close();

    FILE* fptr;
    fptr = fopen ("outputfile1.txt","w");

    int datasent[n+1];



/* --------DECLARING THREADS---------*/

    pthread_t creator_threads[2];
    pthread_t node_threads[n];


/* --------INITIALISING ARRAY TO PASS PARAMETERS---------*/

    int sockarr1[n*n];

    argument_class_creator arguments1;                 // Array to pass the parameters to server thread
    arguments1.socketarray=sockarr1;
    arguments1.n=n;

    int sockarr2[n*n];

    argument_class_creator arguments2;                 // Array to pass the parameters to client thread
    arguments2.socketarray=sockarr2;
    arguments2.n=n;


/* --------CREATING ONE CLIENT AND ONE SERVER THREAD---------*/

    pthread_create(&creator_threads[0], NULL, creator_func1, (void*)&arguments1);                   // Creating server thread
    pthread_create(&creator_threads[1], NULL, creator_func2, (void*)&arguments2);                   // Creating client thread



    for (int j=0;j<2;j++)
    {
        pthread_join(creator_threads[j], NULL);                                                      // Joining threads
    }


/* --------INITIALISING ARRAY TO PASS PARAMETERS---------*/

    argument_class_node arguments[n];                 // Array to pass the parameters to node threads
    for (int j=0;j<n;j++)
    {
        arguments[j].j=j;
        arguments[j].n=n;
        arguments[j].sockarr1=sockarr1;
        arguments[j].sockarr2=sockarr2;
        arguments[j].lambda=lambda;
        arguments[j].alpha=alpha;
        arguments[j].m=m;
        arguments[j].fptr=fptr;
        arguments[j].ptr=datasent;
        arguments[j].adj_list=adjacencylist;
    }


/* --------CREATING N NODE THREADS---------*/

    for (int i=0;i<n;i++){
        pthread_create(&node_threads[i], NULL, node_func, (void*)&arguments[i]);               // Creating threads
    }

    for (int j=0;j<n;j++){
        pthread_join(node_threads[j], NULL);                                                      // Joining threads
    }



/* --------PRINTING SPACE USAGE OUTPUT TO CONSOLE AND LOG FILE---------*/

    datasent[n]=0;
    for(int i=0;i<n;i++){
        datasent[n]+=datasent[i];
        printf("\nProcess %d\nSpace utilised by to store\ni) Vector Clocks: %lu bytes\nSize of the messages sent: %d bytes\nNumber of entries sent: %d\n"
           ,i+1,n*sizeof(int),datasent[i],n*m);
        fprintf(fptr,"\nProcess %d\nSpace utilised by to store\ni) Vector Clocks: %lu bytes\nSize of the messages sent: %d bytes\nNumber of entries sent: %d\n"
           ,i+1,n*sizeof(int),datasent[i],n*m);
    }

    printf("\n\nTotal space utilised to store\ni) Vector Clocks: %lu bytes\nTotal size of the messages sent: %d bytes\nTotal number of entries sent: %d"
           ,n*n*sizeof(int),datasent[n],n*n*m);
    fprintf(fptr,"\n\nTotal space utilised to store\ni) Vector Clocks: %lu bytes\nTotal size of the messages sent: %d bytes\nTotal number of entries sent: %d"
           ,n*n*sizeof(int),datasent[n],n*n*m);

    fclose(fptr);

    return 0;
}
