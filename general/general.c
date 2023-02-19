#include "./general.h"

int mandaRetorno(int isAck, int soquete, int sequencia)
{
    msgT mensagem;
    if (isAck)
    {
        initMessage(&mensagem, NULL, 1, ACK, sequencia);
    }
    else
    {
        initMessage(&mensagem, NULL, 1, ACK, sequencia);
    }
    if (!sendMessage(soquete, &mensagem))
    {
        perror("Erro ao enviar mensagem de retorno");
        return -1;
    }
    return 0;
}


/**********************ERROR*****************************************************************************************/

void getNextState(typesState curState, unsigned int receivedBit, tNode *nextNode)
{
    // CHECK WHAT IS THE CURRENT STATES
    // printf("anterior: %d receiv: %d ",curState, receivedBit);

    if (curState == A)
    {
        if (receivedBit == 0)
        {
            nextNode->curState = A;
            strcpy(nextNode->correctedBits, "00\0");
            // printf("A -> A corrigido %s \n", nextNode->correctedBits);
        }
        else
        {
            nextNode->curState = C;
            strcpy(nextNode->correctedBits, "11\0");
            // printf("A -> C corrigido %s \n", nextNode->correctedBits);
        }
    }
    if (curState == B)
    {
        if (receivedBit == 0)
        {
            nextNode->curState = A;
            strcpy(nextNode->correctedBits, "11\0");
            // printf("B -> A corrigido %s \n", nextNode->correctedBits);
        }
        else
        {
            nextNode->curState = C;
            strcpy(nextNode->correctedBits, "00\0");
            // printf("B -> C corrigido %s \n", nextNode->correctedBits);
        }
    }
    if (curState == C)
    {
        if (receivedBit == 0)
        {
            nextNode->curState = B;
            strcpy(nextNode->correctedBits, "10\0");
            // printf("C -> B corrigido %s \n", nextNode->correctedBits);
        }
        else
        {
            nextNode->curState = D;
            strcpy(nextNode->correctedBits, "01\0");
            // printf("C -> D corrigido %s \n", nextNode->correctedBits);
        }
    }
    if (curState == D)
    {
        if (receivedBit == 0)
        {
            nextNode->curState = B;
            strcpy(nextNode->correctedBits, "01\0");
            // printf("D -> B corrigido %s \n", nextNode->correctedBits);
        }
        else
        {
            nextNode->curState = D;
            strcpy(nextNode->correctedBits, "10\0");
            // printf("D -> D corrigido %s \n", nextNode->correctedBits);
        }
    }
}

unsigned int calcHanningDistance(bit *receivedPacketMessage, bit *correctedPacketMessage, unsigned int packageSize)
{
    unsigned int hanningDist = 0;
    for (unsigned int i = 0; i < packageSize; ++i)
        if (receivedPacketMessage[i] != correctedPacketMessage[i])
            ++hanningDist;
    return (hanningDist);
}

/** VITERBI ALGORITHM **/

void updatePathError(tNode *root, unsigned int level, tListNode **auxList, unsigned int height, bit *receivedStepMessage, unsigned int packetSize)
{
    if (root == NULL)
        return;
    if (level == 0)
    {
        insertfirst(root, auxList);
        root->pathError += calcHanningDistance(receivedStepMessage, root->correctedBits, packetSize);
    }
    else if (level > 0)
    {
        if (root->shouldContinue)
        {
            updatePathError(root->left, level - 1, auxList, height, receivedStepMessage, packetSize);
            updatePathError(root->right, level - 1, auxList, height, receivedStepMessage, packetSize);
        }
    }
}

tNode *getMinHanningDistancePathLeaf(tNode *root, unsigned int packetSize)
{
    tNode *minHanningDistancePathNode = NULL;
    getListLeafsHannigPathDistance(root, height(root), packetSize, height(root), &minHanningDistancePathNode);
    return minHanningDistancePathNode;
}

void cutLeafs(tListNode *head, unsigned int tam)
{
    tListNode *aux = head;
    for (int i = 0; i < tam; ++i)
    {
        tListNode *tmp = aux->next;
        for (int k = i; k < tam - 1; ++k)
        {
            if (k != i && tmp->value->curState == aux->value->curState && aux->value->pathError < tmp->value->pathError)
            {
                tmp->value->shouldContinue = 0;
            }
            tmp = tmp->next;
        }
        aux = aux->next;
    }
}

bit *viterbiAlgorithm(bit *receivedMessage, unsigned int packetSize, unsigned int msgSize)
{
    tNode *pathRoot = startNode(0, 0, A, 0, packetSize, NULL);
    tNode *minHanningDistancePathAux;
    pathRoot->correctedBits[0] = '0';
    pathRoot->correctedBits[1] = '0';

    unsigned int totalPackages = msgSize / packetSize;
    tListNode *auxList = NULL;

    for (unsigned int i = 0; i < totalPackages; ++i)
    {
        bit receivedStepMessage[packetSize + 1];
        strncpy(receivedStepMessage, &receivedMessage[i * packetSize], packetSize);
        getNextStep(pathRoot, packetSize);
        updatePathError(pathRoot, height(pathRoot), &auxList, height(pathRoot), receivedStepMessage, packetSize);

        cutLeafs(auxList, listSize(auxList)); // cutting leafs which collides in state and path error is bigger
        deleteList(&auxList);
    }

#ifdef DEBUG
    printLevelOrder(pathRoot);
#endif
    minHanningDistancePathAux = getMinHanningDistancePathLeaf(pathRoot, packetSize);
    tListNode *head = NULL;

    while (minHanningDistancePathAux->parent)
    {
        insertfirst(minHanningDistancePathAux, &head);
        minHanningDistancePathAux = minHanningDistancePathAux->parent;
    }
    // prnList(head);

    bit *decodedMessage = getDecodedMessage(head, msgSize);
#ifdef DEBUG
    prnList(head);
#endif

    // deleteList(&head);
    // free_binary_tree(pathRoot);
    return decodedMessage;
}

/**********************END_ERROR*****************************************************************************************/

/**********************GENERATE_MESSAGE**********************************************************************************/

/** TRELLIS ENCODING **/
void trellisShift(bit *trellis, bit newBit)
{
    trellis[2] = trellis[1];
    trellis[1] = trellis[0];
    trellis[0] = newBit;
}

bit encodedX1(bit *trellis)
{
    bit x1;
    x1 = INTTOBIT((BITTOINT(trellis[0]) + BITTOINT(trellis[1]) + BITTOINT(trellis[2])) % 2); // x1 = m xor m1 xor m2
    return x1;
}

bit encodedX2(bit *trellis)
{
    bit x2;
    x2 = INTTOBIT((BITTOINT(trellis[0]) + BITTOINT(trellis[2])) % 2); // x2 = m xor m2
    return x2;
}

void trellisEncode(bit *encodedMessage, bit *originalMessage, unsigned int size)
{
    bit trellis[4] = "000";
    trellis[3] = '\0';

    unsigned int encodCounter = 0;
    for (unsigned int i = 0; i < size; ++i)
    {
        trellisShift(trellis, originalMessage[i]);
        encodedMessage[encodCounter] = encodedX1(trellis);
        encodedMessage[encodCounter + 1] = encodedX2(trellis);
        encodCounter += 2;
    }
    encodedMessage[encodCounter] = '\0';
}

bit calculaParidade(bit *conteudo, unsigned int tam)
{
    // Faz um XOR dos bits para cada byte da mensagem.
    bit aux = conteudo[0];
    for (int i = 1; i < tam; i++)
    {
        aux = aux ^ conteudo[i];
    }

    return aux;
}

void initMessage(msgT *mensagem, bit *originalMessage, unsigned int size, typesMessage msgType, int sequencia)
{
    printf("pre encoded message; %s\n", originalMessage);

    trellisEncode(mensagem->dados, originalMessage, size);
    printf("encoded message; %s \n", mensagem->dados);
    mensagem->paridade = calculaParidade(mensagem->dados, size);
    mensagem->marc_inicio = MARC_INICIO;
    mensagem->tipo = msgType;
    mensagem->tam_msg = strlen(mensagem->dados); // duplica por causa da modulção da trelica
    mensagem->sequencia = sequencia;
}

/**********************END_GENERATE_MESSAGE**********************************************************************************/

/**********************LIST**************************************************************************************************/

void insertfirst(tNode *element, tListNode **head)
{
    tListNode *New;
    New = (tListNode *)malloc(sizeof(tListNode));
    New->value = element;
    New->next = (*head);
    (*head) = New;
}

/* Function to delete the entire linked list */
void deleteList(tListNode **head_ref)
{
    /* deref head_ref to get the real head */
    tListNode *current = *head_ref;
    tListNode *next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    /* deref head_ref to affect the real head back
       in the caller. */
    *head_ref = NULL;
}

bit *getDecodedMessage(tListNode *head, unsigned int decodedMsgSize)
{
    bit *decodedMsg = (bit *)malloc(sizeof(bit) * (decodedMsgSize + 1)); // since message will be encoded to x1 and x2, it will be double the message

    tListNode *aux = head;
    unsigned int i = 0;
    while (aux != NULL)
    {
        tNode *value = aux->value;
        decodedMsg[i] = INTTOBIT(value->receivedBit);
        aux = aux->next;
        ++i;
    }
    decodedMsg[i] = '\0';
    return decodedMsg;
}

unsigned int listSize(tListNode *head)
{
    tListNode *aux = head;
    unsigned int size = 0;
    while (aux != NULL)
    {
        ++size;
        aux = aux->next;
    }

    return size;
}

void prnList(tListNode *head)
{
    tListNode *aux = head;
    while (aux != NULL)
    {
        printf("(%s, %d, %d)", aux->value->correctedBits, aux->value->receivedBit, aux->value->shouldContinue);
        aux = aux->next;
    }

    printf("\n");
}

/**********************END_LIST**************************************************************************************************/

/**********************BINARY_TREE*******************************************************************************************/

void copyPathAggregatedMessage(bit *prevPathMess, bit *levelMessage, bit *pathAggMess, unsigned int level, unsigned int packetSize)
{
    for (int i = 0; i < (packetSize * level - 1); ++i)
    {
        pathAggMess[i] = prevPathMess[i];
    }
    for (int i = packetSize * level - 1; i < packetSize * level; ++i)
    {
        pathAggMess[i] = levelMessage[i - packetSize * level];
    }
};

tNode *startNode(unsigned int curPathError, unsigned int inputBit, typesState curState, unsigned int level, unsigned int packetSize, tNode *parentNode)
{
    tNode *n = (tNode *)malloc(sizeof(tNode));

    n->shouldContinue = 1;
    n->pathError = curPathError;
    n->left = NULL;
    n->right = NULL;
    n->receivedBit = inputBit;
    getNextState(curState, inputBit, n); // get next state according to the trellice diagram
    n->parent = parentNode;
    return n;
}

void free_binary_tree(tNode *root)
{
    if (root == NULL)
        return;
    free_binary_tree(root->left);
    free_binary_tree(root->right);
    free(root);
}

/****/
unsigned int countNodes(tNode *n)
{
    if (n != NULL)
    {
        return countNodes(n->left) + countNodes(n->right) + 1;
    }
    else
        return 0;
}

void emordem(tNode *no)
{
    if (no != NULL)
    {
        emordem(no->left);
        printf("%s  ", no->correctedBits);
        emordem(no->right);
    }
}

/* Function to print level order traversal a tree*/
void printLevelOrder(tNode *root)
{
    int h = height(root);
    printf("\n");
    int i;
    for (i = 0; i <= h; i++)
    {
        printCurrentLevel(root, i);
        printf("\n");
    }
}

void getNextStep(tNode *root, unsigned int packetSize)
{
    int h = height(root);
    getNextLeafOnLevel(root, h, packetSize, h);
}

void getNextLeafOnLevel(tNode *root, int level, unsigned int packetSize, unsigned int height)
{
    if (root == NULL)
        return;
    if (level == 0)
    {
        if (root->shouldContinue)
        {
            root->left = startNode(root->pathError, 0, root->curState, height, packetSize, root);
            root->right = startNode(root->pathError, 1, root->curState, height, packetSize, root);
        }
    }
    else if (level > 0)
    {
        getNextLeafOnLevel(root->left, level - 1, packetSize, height);
        getNextLeafOnLevel(root->right, level - 1, packetSize, height);
    }
}

void getListLeafsHannigPathDistance(tNode *root, int level, unsigned int packetSize, unsigned int height, tNode **minHanningDistPathNode)
{
    if (root == NULL)
        return;
    if (level == 0)
    {
        if ((*minHanningDistPathNode) != NULL)
        {
            if (root->pathError < (*minHanningDistPathNode)->pathError)
                (*minHanningDistPathNode) = root;
        }
        else
        {
            (*minHanningDistPathNode) = root;
        }
    }
    else if (level > 0)
    {
        getListLeafsHannigPathDistance(root->left, level - 1, packetSize, height, minHanningDistPathNode);
        getListLeafsHannigPathDistance(root->right, level - 1, packetSize, height, minHanningDistPathNode);
    }
}

/* Print nodes at a current level */
void printCurrentLevel(tNode *root, int level)
{
    if (root == NULL)
        return;
    if (level == 0)
    {
        if (root->parent != NULL)
        {
            if (root->parent->curState == A)
                printf("A->");
            if (root->parent->curState == B)
                printf("B->");
            if (root->parent->curState == C)
                printf("C->");
            if (root->parent->curState == D)
                printf("D->");
        }

        if (root->curState == A)
            printf("A");
        if (root->curState == B)
            printf("B");
        if (root->curState == C)
            printf("C");
        if (root->curState == D)
            printf("D");

        printf("(%s, %d)  |%d| ", root->correctedBits, root->receivedBit, root->pathError);
    }
    else if (level > 0)
    {
        printCurrentLevel(root->left, level - 1);
        printCurrentLevel(root->right, level - 1);
    }
}
/****/

void getFullMessageDecoded(tNode *leaf)
{
    if (leaf->parent == NULL)
        return;
    else
    {
        printf("%s ", leaf->parent->correctedBits);
        getFullMessageDecoded(leaf->parent);
    }
}
/****/
unsigned int height(tNode *p)
{
    int he, hd;
    if (p == NULL)
        return -1;
    he = height(p->left);
    hd = height(p->right);
    if (he > hd)
        return he + 1;
    else
        return hd + 1;
}

/**********************END_BINARY_TREE*******************************************************************************************/

/**********************UTILS*****************************************************************************************************/

int sendMessage(int soquete, msgT *mensagem)
{
    if (send(soquete, mensagem, sizeof(msgT), 0) < 0)
    {
        return 0;
    }
    else
    {
        // error sending message...
        return 1;
    }
}

int ConexaoRawSocket(char *device)
{
    int soquete;
    struct ifreq ir;
    struct sockaddr_ll endereco;
    struct packet_mreq mr;

    soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); /*cria socket*/
    if (soquete == -1)
    {
        printf("Erro no Socket\n");
        exit(-1);
    }

    memset(&ir, 0, sizeof(struct ifreq)); /*dispositivo eth0*/
    memcpy(ir.ifr_name, device, sizeof(char *));
    if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1)
    {
        printf("Erro no ioctl\n");
        exit(-1);
    }

    memset(&endereco, 0, sizeof(endereco)); /*IP do dispositivo*/
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ir.ifr_ifindex;
    if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1)
    {
        printf("Erro no bind\n");
        exit(-1);
    }

    memset(&mr, 0, sizeof(mr)); /*Modo Promiscuo*/
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
    {
        printf("Erro ao fazer setsockopt\n");
        exit(-1);
    }

    return soquete;
}

/**********************END_UTILS*****************************************************************************************************/

int recebe_mensagem(int soquete, msgT *mensagem, int timeout)
{
    msgT mensagemAux; 
    initMessage(&mensagemAux, "0", 1,INIT, 666);

    while (1)
    {
        // cuida do timeout
        struct pollfd fds;

        fds.fd = soquete;
        fds.events = POLLIN;

        int retorno_poll = poll(&fds, 1, TIMEOUT);
        if(retorno_poll)
            printf("\nretorno_poll = %d\n", retorno_poll);

        if (timeout)
        {
            if (retorno_poll == 0)
                return TIMEOUT_RETURN;
            else if (retorno_poll < 0)
                return 0;
        }


        if (recv(soquete, &mensagemAux, sizeof(msgT), 0) < 0)
        {
            return 0;
        }
        else if (mensagemAux.sequencia == sequencia_global){
            printf("sao igausi %d %d\n", mensagemAux.sequencia,sequencia_global);
            // viterbiAlgorithm(mensagemAux.dados,2,mensagemAux.tam_msg);
            // if retorno_pull > 0 entao recebeu alguma mensagem, senao continua
            // printf("seq: %d %s\n",mensagemAux.sequencia, mensagemAux.dados);
            printf("viterbi %s \n",viterbiAlgorithm(mensagemAux.dados,2,mensagemAux.tam_msg)); 
            return 1; // adicionar?
        }
    }
}

void incrementaSequencia()
{
    if (sequencia_global < 15)
        sequencia_global++;
    else
        sequencia_global = 1;
}