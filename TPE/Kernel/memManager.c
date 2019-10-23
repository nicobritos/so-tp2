#include "include/memManager.h"

Node createNode(void * address, uint64_t size, uint64_t pid);
void * buddyAlloc(uint64_t size, uint64_t pid);
void * recursiveBuddyAlloc(uint64_t size, uint64_t pid, uint64_t childBlockSize, Node * parent, uint64_t parentBlockSize);
void createLeftChild(Node * parent, uint64_t size, uint64_t pid);
void createRightChild(Node * parent, uint64_t childsBlockSize, uint64_t size, uint64_t pid);
// void recPrint(Node * node, int depth);
Node * nextFreeSpaceForNode();
bool pfreeRec(uint64_t pid, void * address, Node * current);

Node * root = TREE_ADDRESS; 
bool isOccupiedNodeSpace[MAX_NODES];
uint64_t lastUsedIndex; // Of the isOccupiedNodeSpace array


void initializeMemoryManager(){
    isOccupiedNodeSpace[0] = TRUE;
    lastUsedIndex = 0;
    for (uint64_t i = 1; i < MAX_NODES; i++)
    {
        isOccupiedNodeSpace[i] = FALSE;
    }
    
	Node rootNode = createNode(MEM_STARTING_ADDRESS, sizeof(*root), 0);
    memcpy(root, &rootNode, sizeof(*root));
}

/* Auxiliar, constructor de nodo */
Node createNode(void * address, uint64_t size, uint64_t pid){
    Node node;
    node.size = size;
    node.address = address;
    node.pid = pid;
    node.left = NULL;
    node.right = NULL;
    node.isFull = FALSE;
    return node;
}

void * pmalloc(uint64_t size, uint64_t pid){
    return buddyAlloc(size, pid); // Marca la direccion como ocupada en el arbol del buddy
}

void * buddyAlloc(uint64_t size, uint64_t pid){
    uint64_t childBlockSize;
    for (childBlockSize = MIN_BLOCK_SIZE; childBlockSize < size; childBlockSize *= 2); // Me fijo cual es la menor potencia de 2 que es mayor al size
    return recursiveBuddyAlloc(size, pid, childBlockSize, root, MEM_SIZE);
}

void * recursiveBuddyAlloc(uint64_t size, uint64_t pid, uint64_t childBlockSize, Node * parent, uint64_t parentBlockSize){
    if(parentBlockSize <= childBlockSize) // Me pasé
        return NULL;
    
    if(parent->left == NULL){
        createLeftChild(parent, size, pid);
        parentBlockSize /= 2;
        Node * parentsParent = parent;
        parent = parent->left;
        while(parentBlockSize > childBlockSize){ // Creo siempre a la izquierda hasta llegar al tamaño
            createLeftChild(parent, size, pid);
            parent = parent->left;
            parentBlockSize /= 2;
        }
        parent->isFull = TRUE;
        if(parentsParent->left == parent)
            parentsParent->isFull = TRUE;
        return parent->address;
    }
    if(!(parent->left->isFull)){
        void * ans = recursiveBuddyAlloc(size, pid, childBlockSize, parent->left, parentBlockSize/2);
        if(ans != NULL){
            if(parent->right != NULL && parent->right->isFull && parent->left->isFull){
                parent->isFull = TRUE;
            }
        }
        return ans;
    }
    if(parent->right == NULL){
        createRightChild(parent, parentBlockSize/2, size, pid);
        parentBlockSize /= 2;
        Node * parentsParent = parent;
        parent = parent->right;
        while(parentBlockSize > childBlockSize){ // Creo siempre a la izquierda hasta llegar al tamaño
            createLeftChild(parent, size, pid);
            parent = parent->left;
            parentBlockSize /= 2;
        }
        parent->isFull = TRUE;
        if(parentsParent->right == parent)
            parentsParent->isFull = TRUE;
        return parent->address;
    }
    if(!(parent->right->isFull)){
        void * ans = recursiveBuddyAlloc(size, pid, childBlockSize, parent->right, parentBlockSize/2); //ACASDASDASDASDASDASDASDASDASDASDA
        if(ans != NULL){
            if((parent->right->left != NULL) && (parent->right->right != NULL) && (parent->right->left->isFull) && (parent->right->right->isFull)){
                parent->right->isFull = TRUE;
                parent->isFull = TRUE; // YA CHEQUE ANTES QUE LEFT ESTABA FULL
            }
        }
        return ans;
    }
    return NULL; // no tengo espacio
}

void createLeftChild(Node * parent, uint64_t size, uint64_t pid){
    Node child = createNode(parent->address, size, pid); //primera mitad
    void * nodeSpace = nextFreeSpaceForNode();
    memcpy(nodeSpace, &child, sizeof(child));
    parent->left = nodeSpace;
}

void createRightChild(Node * parent, uint64_t childsBlockSize, uint64_t size, uint64_t pid){
    Node child = createNode(parent->address + childsBlockSize, size, pid); //segunda mitad
    void * nodeSpace = nextFreeSpaceForNode();
    memcpy(nodeSpace, &child, sizeof(child));
    parent->right = nodeSpace;
}

Node * nextFreeSpaceForNode(){
    uint64_t i;
    for (i = (lastUsedIndex + 1) % MAX_NODES; i != lastUsedIndex; i = (i + 1) % MAX_NODES)
    {
        if(isOccupiedNodeSpace[i] == FALSE){
            isOccupiedNodeSpace[i] = TRUE;
            return root + i;
        }
    }
    if(isOccupiedNodeSpace[i] == FALSE){ // Por si se libero el ultimo espacio reservado
        return root + i;
    }
    return NULL;
}

// void printTree(){
//     recPrint(root, 0);
// }

// void recPrint(Node * node, int depth){
//     printf("address = %p - pid = %" PRId64 " - isFull = %d\n", node->address, node->pid, node->isFull);
    
//     if(node->left != NULL){
//         for (int i = 0; i < depth; i++)
//         {
//             printf("\t");
//         }
//         printf("LEFT CHILD - ");
//         recPrint(node->left, depth + 1);
//     }
    
//     if(node->right != NULL){
//         for (int i = 0; i < depth; i++)
//         {
//             printf("\t");
//         }
//         printf("RIGHT CHILD - ");
//         recPrint(node->right, depth + 1);
//     }

// }

void pfree(uint64_t pid, void * address){
    pfreeRec(pid, address, root);
}

bool pfreeRec(uint64_t pid, void * address, Node * current){
    if(current == NULL)
        return FALSE;
    if((current->left == NULL) && (current->right == NULL)){
        if((pid == current->pid) && (address == current->address)){
            return TRUE; // liberar
        }
        return FALSE; // no liberar
    }

    if(current->left == NULL){
        if(pfreeRec(pid, address, current->right)){
            current->right = NULL;
            return TRUE;
        }
    }
    if(current->right == NULL){
        if(pfreeRec(pid, address, current->left)){
            current->left = NULL;
            return TRUE;
        }
    }
    if(pfreeRec(pid, address, current->left) == FALSE){
        if(pfreeRec(pid, address, current->right)){
            current->right = NULL;
        }
    }
    else{
        current->left = NULL;
    }
    if(current->isFull){
        if((current->left == NULL || !(current->left->isFull)) || (current->right == NULL || !(current->right->isFull)))
            current->isFull = FALSE;
    }
    return FALSE;
    
}
