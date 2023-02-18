
int sendMessage(int soquete, msgT *mensagem){
    if (send(soquete, mensagem, sizeof(msgT), 0) < 0){
        return 0;
	}else{
        // error sending message... 
        return 1;
    }
 }