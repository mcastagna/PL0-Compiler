//Mike Castagna

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 5
#define MAX_SYMBOLS 500
#define CODE_SIZE 200

typedef enum {
nulsym = 1, identsym, numbersym, plussym, minussym,
multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
periodsym, becomessym, beginsym, endsym, ifsym, thensym,
whilesym, dosym, callsym, constsym, intsym, procsym, outsym,
insym , elsesym
} token_type;

typedef enum
{
    LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SIO
} operators;

typedef enum
{
    RET = 0, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
} oprvals;

typedef struct opCode {
    int op;    // opcode
    int  l;     // L
    int  m;   // M
}instruction;

typedef struct symbol{
    char id[200];
    struct symbol* next;
} identifier;

typedef struct sym
{
	int kind; 		// const = 1, int = 2, proc = 3
	char* name;	// name up to 11 chars
	int val; 		// number (ASCII value)
	int level; 		// L level
	int addr; 		// M address
} symbolRec;

typedef struct code_table
{
    int op;
    int lex;
    int m;
}code;

//Create symbol table and indices
symbolRec table[MAX_SYMBOLS];
int symIndex = 0;
int pdx = 0;

//Create list of operations
code operations[CODE_SIZE];
int cx = 0;
int lev = 0;

int err = -1;
int err2 = 0;

FILE* output;

//Determines error type and prints
void error(int code)
{
    if (code == 0)
        fprintf(output, "Missing identifier.\n\n");
    else if (code == 1)
        fprintf(output, "Identifier should be followed by =\n\n");
    else if (code == 2)
        fprintf(output, "= should be followed by a number.\n\n");
    else if (code == 3)
        fprintf(output, "Declaration should end with a ;\n\n");
    else if(code == 4)
        fprintf(output, ":= missing in statement\n\n");
    else if(code == 5)
        fprintf(output, "begin must be closed with end\n\n");
    else if(code == 6)
        fprintf(output, "if condition must be followed by then\n\n");
    else if(code == 7)
        fprintf(output, "while condition must be followed by do\n\n");
    else if(code == 8)
        fprintf(output, "relational operator missing in conditional statement\n\n");
    else if(code == 9)
        fprintf(output, "left ( has not been closed\n\n");
    else if(code == 10)
        fprintf(output, "identifier ( or number expected\n\n");
    else if(code == 11)
        fprintf(output, "No period at end of file.\n\n");
    else if(code == 12 || code == 13)
        fprintf(output, "Identifier not declared.\n\n");
    else if(code == 14)
        fprintf(output, "Cannot assign to constant or procedure.\n\n");
    else if(code == 16)
        fprintf(output, "Begin followed by improper statement.\n\n");

}

//Adds operations to code stack
void emit(int op, int l, int m, int i)
{
    if(cx > CODE_SIZE)
        {error(12); err = i; err2 = 12;}
    else
    {
        operations[cx].op = op;
        operations[cx].lex = l;
        operations[cx].m = m;
        cx++;
    }
}

//Adds symbol to symbol table
void makeSymbol(int kind, char* name, int level, int val)
{
    if (kind == 3)
    {
        if (getLevel(name) != -1)
        {
            int index = 0;
            int i;
            for(i = 0; i < symIndex; i++)
            {
                if (strcmp(name, table[i].name) == 0)
                    index = i;
            }

            table[index].level = level;

            return;
        }
    }
    table[symIndex].kind = kind;
    table[symIndex].name = name;
    table[symIndex].level = level;
    table[symIndex].val = val;

    if (kind == 2)
    {
        table[symIndex].val = pdx;
        pdx++;
    }

    symIndex++;
}

int base(int l,int base, int* stack) // l stand for L in the instruction format
{
  int b1; //find base L levels down
  b1 = base;
  while (l > 0)
  {
    b1 = stack[b1 + 1];
    l--;
  }
  return b1;
}

//prints out current level of stack
void printStack(FILE* out, int* stack, int ptr, int* levelVals, int lvl)
{
    int i;
    int j = 0;

    for (i = 1; i <= ptr; i++)
    {
        fprintf(out, "%d ", stack[i]);
        //prints bracket if there are multiple lexical levels
        if (i == levelVals[j])
        {
            fprintf(out, "| ");
            j++;
        }
    }

    fprintf(out, "\n");
}

//returns the name of the command based on the op number
char* getName(int opCode)
{
    char* opName;

            switch (opCode)
        {
            case 1:
                opName = "lit";
                break;
            case 2:
                opName = "opr";
                break;
            case 3:
                opName = "lod";
                break;
            case 4:
                opName = "sto";
                break;
            case 5:
                opName = "cal";
                break;
            case 6:
                opName = "inc";
                break;
            case 7:
                opName = "jmp";
                break;
            case 8:
                opName = "jpc";
                break;
            case 9:
                opName = "sio";
                break;
            case 10:
                opName = "sio";
                break;
        }

        return opName;
}

void makeStack(){
    //open input and output file
    FILE *input;

    input = fopen("intermediate.txt", "r");

    //define array of structs to 500
    int i = 0;
    instruction instr[MAX_CODE_LENGTH];

    fprintf(output, "Line\tOP\tL\tM\t\n");

    //inputs the commands from a file into the stack until the end of the file is reached
    //also prints out commands to file
    while(!feof(input)){
        fscanf(input, "%d%d%d", &instr[i].op, &instr[i].l, &instr[i].m);

        if (!feof(input))
            fprintf(output, "%d\t%s\t%d\t%d\n", i, getName(instr[i].op), instr[i].l, instr[i].m);
        else
            fprintf(output, "\n\n");
        i++;

    }

    //initialize variables
    int stack[MAX_STACK_HEIGHT];
    int cal = -1;
    int sp = 0;
    int bp = 1;
    int ir = 0;
    int pc = 0;
    stack[1] = 0;
    stack[2] = 0;
    stack[3] = 0;

    int level = 0;
    int levelVals[MAX_LEXI_LEVELS];

    //initialize array for lexical levels to -1
    for (i = 0; i < MAX_LEXI_LEVELS; i++)
    {
        levelVals[i] = -1;
    }

    //print formatted categories
    fprintf(output, "\t\t\t\tpc\tbp\tsp\tstack\n");
    fprintf(output, "Initial Values\t\t\t0\t1\t0\n");

    //iterates through commands until the base pointer = 0
    while(bp != 0){
        //initialize variables to current command
        int op = instr[pc].op;
        int m = instr[pc].m;
        int l = instr[pc].l;

        //print current command
        fprintf(output, "%d\t%s\t%d\t%d\t", pc, getName(op), l, m);
        //increment program counter
        pc++;

        //for op 1, add 1 to stack pointer and place m into the current sp
        if (op == 1)
        {
            sp++;
            stack[sp] = m;
        }
        //for op 2, execute specified arithmetic command
        else if (op == 2)
        {
            //return operation
            if(m == 0){
                sp = bp -1;
                pc = stack[sp + 4];
                bp = stack[sp + 3];
                level--;
            }
            //negate current sp
            else if(m == 1){
                stack[sp] = -stack[sp];
            }
            //decrement stack pointer and add the integer on top of the stack into the previous slot in the stack
            else if(m == 2){
                sp--;
                stack[sp] += stack[sp + 1];
            }
            //decrement stack pointer and subtract the integer on top of the stack into the previous slot in the stack
            else if(m == 3){
                sp--;
                stack[sp] -= stack[sp + 1];
            }
            //decrement stack pointer and multiply the integer on top of the stack into the previous slot in the stack
            else if(m == 4){
                sp--;
                stack[sp] *= stack[sp + 1];
            }
            //decrement stack pointer and divide the integer on top of the stack into the previous slot in the stack
            else if(m == 5){
                sp--;
                stack[sp] /= stack[sp + 1];
            }
            //decides if the current stack pointer is odd
            else if(m == 6){
                stack[sp] %= 2;
            }
            //decrement stack pointer and mod the integer on top of the stack into the previous slot in the stack
            else if(m == 7){
                sp--;
                stack[sp] %= stack[sp + 1];
            }
            //decrement stack pointer and return 1 if current integer and the top of the stack are equal
            //else return 0
            else if(m == 8){
                sp--;
                if (stack[sp] == stack[sp + 1])
                    stack[sp] = 1;
                else
                    stack[sp] = 0;
            }
            //decides if the current integer is not equal to the integer on top of the stack
            else if(m == 9){
                sp--;
                if (stack[sp] != stack[sp + 1])
                    stack[sp] = 1;
                else
                    stack[sp] = 0;
            }
            //decides if the current integer is less than the integer on top of the stack
            else if(m == 10){
                sp--;
                if (stack[sp] < stack[sp + 1])
                    stack[sp] = 1;
                else
                    stack[sp] = 0;
            }
            //decides if the current integer is less than or equal to than the integer on top of the stack
            else if(m == 11){
                sp--;
                if (stack[sp] <= stack[sp + 1])
                    stack[sp] = 1;
                else
                    stack[sp] = 0;
            }
            //decides if the current integer is greater than the integer on top of the stack
            else if(m == 12){
                sp--;
                if (stack[sp] > stack[sp + 1])
                    stack[sp] = 1;
                else
                    stack[sp] = 0;
            }
            //decides if the current integer is greater than or equal to the integer on top of the stack
            else if(m == 13){
                sp--;
                if (stack[sp] >= stack[sp + 1])
                    stack[sp] = 1;
                else
                    stack[sp] = 0;
            }
        }
        //op 3, perform the load command
        else if (op == 3)
        {
            sp++;
            stack[sp] = stack[base(l, bp, stack) + m];
        }
        //perform the store command
        else if (op == 4)
        {
            stack[base(l,bp, stack) + m] = stack[sp];
            if ((base(l, bp, stack) + m) < sp)
                sp--;
        }
        //performs the call command
        else if (op == 5)
        {
            if(level > 4){
                pc++;
                continue;
            }

            stack[sp + 1] = 0;
            stack[sp + 2] = base(l,bp, stack);
            stack[sp + 3] = bp;
            stack[sp + 4] = pc;
            bp = sp + 1;
            pc = m;
            levelVals[level] = sp;
            level++;
            sp += 4;
            cal = 1;
        }
        //perform increment command
        else if (op == 6){
            if(cal == 1){
                sp += (m-4);
                cal = -1;
            }
            else
                sp += m;
        }
        //performs the jump command
        else if (op == 7)
            pc = m;
        //performs the jump conditional command
        else if (op == 8)
        {
            if (stack[sp] == 0)
                pc = m;
            else
                sp--;
        }
        //prints current top of stack
        else if (op == 9)
        {
            sp--;
            fprintf(output, "%d\n", stack[sp]);
            sp--;
        }
        //enters a number from the user to the stack
        else if (op == 10)
        {
            sp++;
            printf("Enter a number to be placed on the stack.\n");
            scanf("%d", &stack[sp]);
        }
        //prints the current status of the stack
        fprintf(output, "%d\t%d\t%d\t", pc, bp, sp);
        printStack(output, stack, sp, levelVals, level);
    }
    fclose(input);

    return;

}

int is_alpha(const char c) {
    if( c >= 'a' && c <= 'z' )
        return 1;
    if( c >= 'A' && c <= 'Z' )
        return 1;

    return 0;
}

int is_digit(const char c) {
    if( c >= '0' && c <= '9' )
        return 1;

    return 0;
}

int is_space(const char c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\0' || c == '\r')
        return 1;

    return 0;
}

//Prints out list of lexemes and identifiers
void printLexList(FILE* out, int* lexList, int count, int* numList, int numCount, identifier* idList, int idCount)
{
    int currNum = 0;
    int i;

    identifier* curr = idList;

    for (i = 0; i < count; i++)
    {
        fprintf(out, "%d ", lexList[i]);

        //Prints number or identifier from respective list
        if (lexList[i] == 3)
        {
            fprintf(out, "%d ", numList[currNum]);
            currNum++;
        }
        if (lexList[i] == 2)
        {
            fprintf(out, "%s ", curr->id);
            curr = curr->next;
        }
    }

    fprintf(out, "\n");
}

//Accepts alphanumeric string and determines its symbolic value
int identifyToken(FILE* out, char token[], int length, int* lexList, int* count, int* numList, int* numCount, identifier** idList, int* idCount)
{
    int val = 0;

    //Compare string to all PL/0 reserved words
    if (strcmp(token, "begin") == 0)
        val = 21;
    else if (strcmp(token, "end") == 0)
        val = 22;
    else if (strcmp(token, "if") == 0)
        val = 23;
    else if (strcmp(token, "then") == 0)
        val = 24;
    else if (strcmp(token, "while") == 0)
        val = 25;
    else if (strcmp(token, "do") == 0)
        val = 26;
    else if (strcmp(token, "call") == 0)
        val = 27;
    else if (strcmp(token, "const") == 0)
        val = 28;
    else if (strcmp(token, "int") == 0)
        val = 29;
    else if (strcmp(token, "procedure") == 0)
        val = 30;
    else if (strcmp(token, "out") == 0)
        val = 31;
    else if (strcmp(token, "in") == 0)
        val = 32;
    else if (strcmp(token, "else") == 0)
        val = 33;
    else if (strcmp(token, "odd") == 0)
        val = 8;
    else //If the string is not a reserved word, it is either a number or identifier
    {
        if (is_digit(token[0])) //Case where string is assumed to be a number
        {
            int i;
            for (i = 0; i < length; i++)
                if(!is_digit(token[i])) //Case where string begins with number and contains letters
                {
                    fprintf(output, "Error: Variable cannot start with digit.\n");
                    return -1;
                }

            if (length > 5)
            {
                fprintf(output, "Error: Number cannot exceed 5 digits.\n");
                return -1;
            }
            else //Case where number is of proper format
            {
                val = 3;

                //Converts character digits into integers
                int number = token[0] - '0';
                for(i = 1; i < length; i++)
                {
                    number *= 10;
                    number += token[i] - '0';
                }
                //Number is placed into list of values
                numList[*numCount] = number;

                *numCount = *numCount + 1;
            }
        }
        else if (is_alpha(token[0])) //Case where string is assumed to be an identifier
        {
            if (length > 11)
            {
                fprintf(output, "Error: Identifier cannot exceed 11 characters.\n");
                return -1;
            }

            val = 2;

            //If the identifier is not longer than 11 characters, it is added to a linked list
            identifier* temp = (identifier*)malloc(sizeof(identifier));
            strcpy(temp->id, token);
            temp->next = NULL;

            if (*idList == NULL)
                *idList = temp;
            else
            {
                identifier* curr = *idList;
                while(curr->next != NULL)
                    curr = curr->next;

                curr->next = temp;
            }

        }
    }

    if (val != 0) //Prints the string and its symbolic value if it is well-formed
    {
        lexList[*count] = val;
        *count = *count + 1;
    }
    else
    {
        fprintf(output, "Invalid symbol.\n");
        return -1;
    }

    return val;
}

//Accepts non-alphanumeric character or string and determines its symbolic value
int identifySymbol(FILE* out, char* program, int length, int i, int* lexList, int* count)
{
    //If these two symbols appear together, check to see if there is another pair
    if (i+1 < length && program[i] == '/' && program[i+1] == '*')
    {
        int j = i;
        while (1)
        {
            if(j >= length)
                break;
            if((program[j] == '*' && program[j+1] == '/')) //Jumps lexical analysis position past comment block
                return j + 2;
            j++;
        }

    }

    int val;

    //Determines symbolic value of reserved symbols
    switch (program[i])
    {
        case '+':
            val = 4;
            break;
        case '-':
            val = 5;
            break;
        case '*':
            val = 6;
            break;
        case '/':
            val = 7;
            break;
        case '=':
            val = 9;
            break;
        case '<':
            val = 11;
            break;
        case '>':
            val = 13;
            break;
        case '(':
            val = 15;
            break;
        case ')':
            val = 16;
            break;
        case ',':
            val = 17;
            break;
        case ';':
            val = 18;
            break;
        case '.':
            val = 19;
            break;
        default:
            val = 0;
    }

    if (val == 0 && program[i] != ':' && is_space(program[i+1]))
    {
        fprintf(output, "Invalid symbol\n");
        return -1;
    }

    //Look to the next character to see if there is another symbol that pairs with the current symbol
    if (is_alpha(program[i+1]) || is_digit(program[i+1]) || is_space(program[i+1]))
    {
        lexList[*count] = val;
        *count = *count + 1;
        return i;
    }
    else
    {
        if (program[i] == ':' && program[i+1] == '=')
            val = 20;
        else if (program[i] == '<')
            if (program[i+1] == '>')
                val = 10;
            else if (program[i+1] == '=')
                val = 12;
            else
            {
                lexList[*count] = val;
                *count = *count + 1;
                return i;
            }
        else if (program[i] == '>' && program[i+1] == '=')
            val = 14;
        else
        {
            lexList[*count] = val;
            *count = *count + 1;
            return i;
        }

        lexList[*count] = val;
        *count = *count + 1;
        return i + 1;
    }

    return i;
}

//Functions accepts program as input file and generates lexeme list as output
int scan(FILE* input)
{
    FILE* lex;

    lex = fopen("lexlist.txt", "w");

    if(input == NULL){
        fprintf(output, "File does not exist.\n");
        return -1;
    }

    char file[2000];
    int i = 0;

    while(!feof(input))
    {
        fscanf(input, "%c", &file[i]);
        i++;
    }

//Declare arrays to store symbolic values, numbers, and identifiers
    char program[i-1];
    int lexemeList[i-1];
    int numberList[i-1];
    int currLex = 0;
    int currNum = 0;
    int currID = 0;
    identifier* idList = NULL;

//Copy all of file into properly sized character array
    int j;
    for (j = 0; j < i-1; j++)
        program[j] = file[j];

    char token[200];

    int t = 0;

    //Read each character in program
    for (j = 0; j < i-1; j++)
    {
        if (is_alpha(program[j])) //Letter and numbers are added to current token until symbol or space is reached
        {
            token[t] = program[j];
            t++;
        }
        else if (is_digit(program[j]))
        {
            token[t] = program[j];
            t++;
        }
        else //When symbol or space is reached
        {
            int tokenVal;
            if (t != 0) //If the token is one or more characters in length
            {
                //Add null character to terminate string
                token[t] = '\0';

                //Get symbol value for token
                tokenVal = identifyToken(lex, token, t, lexemeList, &currLex, numberList, &currNum, &idList, &currID);

                //If the token was invalid, the program terminates
                if (tokenVal == -1)
                    return -1;
            }
            if (!is_space(program[j])) //If one symbol has been read that is not white space, get its symbolic value
                j = identifySymbol(lex, program, i-1, j, lexemeList, &currLex);

            //If the symbol was invalid, the program terminates
            if (j == -1)
                return -1;

            t = 0;
        }
    }

    //Print list of lexemes with numbers and identifiers
    printLexList(lex, lexemeList, currLex, numberList, currNum, idList, currID);

    fclose(lex);

    return 1;
}

//Determines if tokens are part of well-formed program
int program(char list[][20], int size)
{
    int lev = 0;
    int i = 0;
    char* token;
    token = list[0];
    emit(INC, lev, 6, i);
    pdx = 4;

    //Determines if program has well-formed block
    i = block(list, size, token, 0);
    if (i == -1)
        return 0;
    token = list[i];

    //Program must end with period
    if (atoi(token) != periodsym)
    {
        error(11); err = i; err2 = 11;
        return 0;
    }

    emit(OPR, 0, RET, i);
    return 1;
}

//Determines if block is well-formed
int block(char list[][20], int size, char* token, int i)
{
    //Case where block begins with constant declaration
    if(atoi(token) == constsym){
        do
        {
            i++;
            token = list[i];

            //Do not accept constant without identifier
            if(atoi(token) != identsym)
            {
                error(0); err = i; err2 = 0;
                return -1;
            }

            char* name = list[i+1];

            i+=2;
            token = list[i];

            //Constant must be followed by equals
            if(atoi(token) != eqsym)
            {
                error(1); err = i; err2 = 1;
                return -1;
            }

            i++;
            token = list[i];

            //Constant must be assigned to a number
            if (atoi(token) != numbersym)
            {
                error(2); err = i; err2 = 2;
                return -1;
            }

            int val = atoi(list[i+1]);
            i+=2;
            token = list[i];

            //If constant is well-formed it is added to symbol table
            if (atoi(token) == commasym || atoi(token) == semicolonsym)
                makeSymbol(1, name, lev, val);

        }while(atoi(token) == commasym); //Continue while more constants exist
        if(atoi(token) != semicolonsym)
        {
            error(3); err = i; err2 = 3;
            return -1;
        }

        i++;
        token = list[i];
    }

    //Determines if int declarations are well-formed
    if(atoi(token) == intsym)
    {
        do{
            i++;
            token = list[i];

            //Int must have identifier
            if(atoi(token) != identsym)
            {
                error(0); err = i; err2 = 0;
                return -1;
            }

            char* name = list[i+1];
            i+=2;
            token = list[i];

            //Adds int to symbol table
            if (atoi(token) == commasym || atoi(token) == semicolonsym)
                makeSymbol(2, name, lev, 0);

        } while(atoi(token) == commasym); //Continues while more int declarations
        if(atoi(token) != semicolonsym)
        {
            error(3); err = i; err2 = 3;
            return -1;
        }

        i++;
        token = list[i];
    }
    while(atoi(token) == procsym) //Accepts all well-formed procedure declarations
    {
        i++;
        token = list[i];

        //Procedure must have identifier
        if(atoi(token) != identsym)
        {
            error(0); err = i; err2 = 0;
            return -1;
        }
        char* name = list[i+1];

        //Add procedure to symbol table
        makeSymbol(3, name, lev, cx+1);
        printf("%s", name);
        i+=2;
        token = list[i];

        //Procedure must end with semicolon
        if(atoi(token) != semicolonsym)
        {
            error(3); err = i; err2 = 3;
            return -1;
        }
        i++;
        token = list[i];

        //Procedure must have a well-formed block
        lev++;
        int tempCX = cx;
        emit(JMP, 0, 0, i);
        i = block(list, size, token, i);
        emit(OPR, 0, 0, i);
        operations[tempCX].m = cx;
        lev--;
        if (i == -1)
            return -1;
        token = list[i];

        //Block must end with semi-colon
        if(atoi(token) != semicolonsym)
        {
            error(3); err = i; err2 = 3;
            return -1;
        }
        i++;
        token = list[i];
    }

    //Block may have a statement
    i = statement(list, size, token, i);
    if (i == -1)
        return -1;

    return i;
}

//Determines if statement is well-formed
int statement(char list[][20], int size, char* token, int i)
{
    //Case where statement begins with identifier
    if (atoi(token) == identsym)
    {
        char* name = list[i+1];
        i+=2;
        token = list[i];

        int kind = getKind(name);

        //Identifier must be assigned a value in this case
        if (atoi(token) != becomessym)
        {
            error(4); err = i; err2 = 4;
            return -1;
        }
        i++;
        token = list[i];

        //Determines if identifier will be assigned to well-formed expression
        i = expression(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        //If identifier is variable, emit intermediate code
         if(kind == 2)
         {
                int level = getLevel(name);
                level = abs(level - lev);
                emit(STO, level, getVal(name), i);
         }
        else if (kind == 1 || kind == 3) //Cannot assign to constant or procedure
        {
            error(14); err = i; err2 = 14;
            return -1;
        }
        else //Cannot assign to identifier not previously declared
        {
            error(13); err = i; err2 = 13;
            return -1;
        }
    }
    else if (atoi(token) == callsym) //Case where statement makes procedure call
    {
        i++;
        token = list[i];

        //Call must be followed by a procedure identifier
        if (atoi(token) != identsym)
        {
            error(10); err = i; err2 = 10;
            return -1;
        }
        else
        {
            token = list[i+1];
            emit(CAL, getLevel(token), getVal(token), i);
            i+=2;
            token = list[i];
        }

    }
    else if (atoi(token) == beginsym) //Case where statement begins procedure
    {
        i++;
        token = list[i];

        //Determines if contained statement is well-formed
        i = statement(list, size, token, i);
        if (i == -1)
            return -1;

        token = list[i];

        //Accepts additional statements
        while(atoi(token) == semicolonsym)
        {
            i++;
            token = list[i];
            i = statement(list, size, token, i);
            if (i == -1)
                return -1;
            token = list[i];


            if (atoi(token) != semicolonsym && atoi(token) != endsym)
            {
                int j = i;

                j = statement(list, size, token, j);
                if (j != -1)
                {
                    error(3); err = i; err2 = 3;
                    return -1;
                }
                else
                {
                    printf("here\n");
                    break;
                }
            }

        }
        if (atoi(token) != endsym) //Begin must have end
        {
            error(5); err = i; err2 = 5;
            return -1;
        }

        i++;
        token = list[i];
    }
    else if (atoi(token) == ifsym) //Case where statement begins with if
    {
        i++;
        token = list[i];

        //If must be followed by well-formed condition
        i = condition(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        //Condition must be followed by then
        if (atoi(token) != thensym)
        {
           error(6); err = i; err2 = 6;
           return -1;
        }
        i++;
        token = list[i];

        //Generates jump code for condition
        int ctemp = cx;
        emit(JPC, 0, 0, i);

        //Determines if conditional statement is well-formed
        i = statement(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        operations[ctemp].m = cx;
    }
    else if (atoi(token) == whilesym) //Case where statement begisn with while
    {
        int cx1 = cx;
        i++;
        token = list[i];

        //Determines if while condition is well-formed
        i = condition(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        //Generates jump code for while loop
        int cx2 = cx;
        emit(JPC, 0, 0, i);

        //Condition must be followed by do
        if (atoi(token) != dosym)
        {
            error(7); err = i; err2 = 7;
            return -1;
        }

        i++;
        token = list[i];

        //While must execute well-formed statement
        i = statement(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        emit(JMP, 0, cx1, i);
        operations[cx2].m = cx;
    }
    else
    {
        //error(16); err = i; err2 = 16;
        //return -1;
    }

    return i;
}

//Determines if condition is well-formed
int condition(char list[][20], int size, char* token, int i)
{
    if (atoi(token) == oddsym) //Case where odd operator is used
    {
        i++;
        token = list[i];

        //Odd must be followed by well-formed expression
        i = expression(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        emit(OPR, 0, ODD, i);
    }
    else
    {
        //Condition must have well-formed expression on left side of operator
        i = expression(list, size, token, i);
        if (i == -1)
            return -1;

        token = list[i];
        char* op = token;
        int oper = atoi(op);

        //Expression must be followed by valid operator
        if (atoi(token) != eqsym && atoi(token) != neqsym && atoi(token) != gtrsym && atoi(token) != geqsym
            && atoi(token) != lessym && atoi(token) != leqsym){
                error(8); err = i; err2 = 8;
                return -1;
        }

        i++;
        token = list[i];

        //Operator must have well-formed expression on right side
        i = expression(list, size, token, i);
        if (i == -1)
            return -1;

        //Generate code for condition
        if (oper == eqsym)
            emit(OPR, 0, EQL, i);
        else if (oper == neqsym)
            emit(OPR, 0, NEQ, i);
        else if (oper == gtrsym)
            emit(OPR, 0, GTR, i);
        else if (oper == geqsym)
            emit(OPR, 0, GEQ, i);
        else if (oper == lessym)
            emit(OPR, 0, LSS, i);
        else if (oper == leqsym)
            emit(OPR, 0, LEQ, i);

        token = list[i];
    }

    return i;
}

//Determines if expression is well-formed
int expression(char list[][20], int size, char* token, int i)
{
    //Case where expression begins with plus or minus
    if(atoi(token) == plussym || atoi(token) == minussym)
    {
        char* addop = token;
        i++;
        token = list[i];

        //Must be followed by well-formed term
        i = term(list, size, token, i);
        if (i == -1)
            return -1;
        if(atoi(addop) == minussym)
            emit(OPR, 0, NEG, i);
    }
    else
    {
        //Otherwise, expression must begin with term
        i = term(list, size, token, i);

        if (i == -1)
            return -1;
    }


    token = list[i];

    //Term may be followed by plus/minus and another well-formed term
    while(atoi(token) == plussym || atoi(token) == minussym)
    {
        char* addop = token;
        i++;
        token = list[i];

        i = term(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        if (atoi(addop) == plussym)
            emit(OPR, 0, ADD, i);
        else
            emit(OPR, 0, SUB, i);
    }

    return i;
}

//Determines if term is well-formed
int term(char list[][20], int size, char* token, int i)
{
    char* mulop;

    //Term must begin with factor
    i = factor(list, size, token, i);
    if (i==-1)
        return -1;
    token = list[i];

    //Term may be followed by multiply/divide and other terms
    while(atoi(token) == multsym || atoi(token) == slashsym)
    {
        mulop = token;
        i++;
        token = list[i];
        i = factor(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];

        if (atoi(mulop) == multsym)
            emit(OPR, 0, MUL, i);
        else
            emit(OPR, 0, DIV, i);
    }

    return i;
}

int getKind(char* token)
{
    int i;
    for(i = 0; i < symIndex; i++)
    {
        if (strcmp(token, table[i].name) == 0)
            return table[i].kind;
    }

    return -1;
}
int getLevel(char* token)
{
    int i;
    for(i = 0; i < symIndex; i++)
    {
        if (strcmp(token, table[i].name) == 0)
            return table[i].level;
    }

    return -1;
}
int getVal(char* token)
{
    int i;
    for(i = 0; i < symIndex; i++)
    {
        if (strcmp(token, table[i].name) == 0)
            return table[i].val;
    }

    return -1;
}

//Determines if factor is well-formed
int factor(char list[][20], int size, char* token, int i)
{
    //Factor may begin with identifier
    if(atoi(token) == identsym)
    {
        char* id = list[i+1];
        if(getKind(id) == 1)
            emit(LIT, getLevel(id), getVal(id), i);
        else if (getKind(id) == 2)
        {
            int level = getLevel(id);
            level = abs(lev - level);
            printf("%d %d\n", level, lev);
            emit(LOD, level, getVal(id), i);
        }
        else if (getKind(id) == -1 || getKind(id) == 3) //Identifier cannot be procedure or undeclared
        {
            error(12); err = i; err2 = 12;
            return -1;
        }

        i+=2;
        token = list[i];
    }
    else if(atoi(token) == numbersym) //Factor may begin with number
    {
        emit(LIT, lev, atoi(list[i+1]), i);
        i+=2;
        token = list[i];
    }
    else if(atoi(token) == lparentsym) //Factor may be an expression contained within paratheses
    {
        i++;
        token = list[i];
        i = expression(list, size, token, i);
        if (i == -1)
            return -1;
        token = list[i];
        if(atoi(token) != rparentsym)
        {
            error(9); err = i; err2 = 9;
            return -1;
        }

        i++;
        token = list[i];
    }
    else
    {
        error(10); err = i; err2 = 10;
        return -1;
    }

    return i;
}

//Converts array of operations into intermediate code file to be read by VM
void generateCode()
{
    FILE* coutput = fopen("intermediate.txt", "w");

    int i;

    for (i = 0; i < cx; i++)
        fprintf(coutput, "%d %d %d\n", operations[i].op, operations[i].lex, operations[i].m);

    fclose(coutput);
    return;
}

//Prints symbolic representation of program
void symbolRepresent(char list[][20], int size)
{
    int i = 0;
    for(i = 0; i <= size; i++)
    {
        if(atoi(list[i]) == 1)
            {if(err != -1) fprintf(output, "nul "); else fprintf(output, "nulsym ");}
        else if(atoi(list[i]) == 2)
            {if(err != -1){i++; fprintf(output, " %s", list[i]);} else fprintf(output, "identsym ");}
        else if(atoi(list[i]) == 3)
            {if(err != -1) {i++; fprintf(output, " %s", list[i]);} else fprintf(output, "numbersym ");}
        else if(atoi(list[i]) == 4)
            {if(err != -1) fprintf(output, " + "); else fprintf(output, "plussym ");}
        else if(atoi(list[i]) == 5)
            {if(err != -1) fprintf(output, " - "); else fprintf(output, "minussym ");}
        else if(atoi(list[i]) == 6)
            {if(err != -1) fprintf(output, " * "); else fprintf(output, "multsym ");}
        else if(atoi(list[i]) == 7)
            {if(err != -1) fprintf(output, " / "); else fprintf(output, "slashsym ");}
        else if(atoi(list[i]) == 8)
            fprintf(output, "oddsym ");
        else if(atoi(list[i]) == 9)
            {if(err != -1) fprintf(output, " = "); else fprintf(output, "eqsym ");}
        else if(atoi(list[i]) == 10)
            {if(err != -1) fprintf(output, " <> "); else fprintf(output, "neqsym ");}
        else if(atoi(list[i]) == 11)
            {if(err != -1) fprintf(output, " < "); else fprintf(output, "lessym ");}
        else if(atoi(list[i]) == 12)
            {if(err != -1) fprintf(output, " <= "); else fprintf(output, "leqsym ");}
        else if(atoi(list[i]) == 13)
            {if(err != -1) fprintf(output, " > "); else fprintf(output, "gtrsym ");}
        else if(atoi(list[i]) == 14)
            {if(err != -1) fprintf(output, " >= "); else fprintf(output, "geqsym ");}
        else if(atoi(list[i]) == 15)
            {if(err != -1) fprintf(output, "("); else fprintf(output, "lparentsym ");}
        else if(atoi(list[i]) == 16)
            {if(err != -1) fprintf(output, ") "); else fprintf(output, "rparentsym ");}
        else if(atoi(list[i]) == 17)
            {if(err != -1) fprintf(output, ", "); else fprintf(output, "commasym ");}
        else if(atoi(list[i]) == 18)
            {if(err != -1) fprintf(output, ";\n"); else fprintf(output, "semicolonsym ");}
        else if(atoi(list[i]) == 19)
            {if(err != -1) fprintf(output, ".\n"); else fprintf(output, "periodsym ");}
        else if(atoi(list[i]) == 20)
            {if(err != -1) fprintf(output, " := "); else fprintf(output, "becomessym ");}
        else if(atoi(list[i]) == 21)
            {if(err != -1) fprintf(output, "begin\n"); else fprintf(output, "beginsym ");}
        else if(atoi(list[i]) == 22)
            {if(err != -1) fprintf(output, "end\n"); else fprintf(output, "endsym ");}
        else if(atoi(list[i]) == 23)
            {if(err != -1) fprintf(output, "if "); else fprintf(output, "ifsym ");}
        else if(atoi(list[i]) == 24)
            {if(err != -1) fprintf(output, " then\n"); else fprintf(output, "thensym ");}
        else if(atoi(list[i]) == 25)
            {if(err != -1) fprintf(output, "while "); else fprintf(output, "whilesym ");}
        else if(atoi(list[i]) == 26)
            {if(err != -1) fprintf(output, "do "); else fprintf(output, "dosym ");}
        else if(atoi(list[i]) == 27)
            {if(err != -1) fprintf(output, "call "); else fprintf(output, "callsym ");}
        else if(atoi(list[i]) == 28)
            {if(err != -1) fprintf(output, "const "); else fprintf(output, "constsym ");}
        else if(atoi(list[i]) == 29)
            {if(err != -1) fprintf(output, "int "); else fprintf(output, "intsym ");}
        else if(atoi(list[i]) == 30)
            {if(err != -1) fprintf(output, "procedure "); else fprintf(output, "procsym ");}
        else if(atoi(list[i]) == 31)
            {if(err != -1) fprintf(output, "out "); else fprintf(output, "outsym ");}
        else if(atoi(list[i]) == 32)
            {if(err != -1) fprintf(output, "in "); else fprintf(output, "insym ");}
        else if(atoi(list[i]) == 33)
            {if(err != -1) fprintf(output, "else "); else fprintf(output, "elsesym ");}
        else
            {if(err != -1) fprintf(output, "%s", list[i]); else fprintf(output, "%s ", list[i]);}

    }
}

int main()
{
    //Opens the program
    FILE* prog = fopen("input.txt", "r");

    //Opens file for program output
    output = fopen("output.txt", "w");
    char list[2000][20];

    //Scans program and converts to lexeme list
    int lexError = scan(prog);

    fclose(prog);

    if (lexError == -1)
        return 0;

    //Opens lexeme list to be parsed
    FILE* input = fopen("lexlist.txt", "r");

    int size = 0;

    //Reads lexeme list
    while(!feof(input))
    {
        fscanf(input, "%s", list[size]);
        size++;
    }

    fprintf(output, "A print out of the token (internal representation) file:\n\n");
    int i;

    for (i = 0; i < size; i++)
        fprintf(output, "%s ", list[i]);

    fprintf(output, "\n\nAnd its symbolic representation:\n\n");

    symbolRepresent(list, size); fprintf(output, "\n\n");

    //Determines if program is syntactically correct and generates intermediate code
    int correct = program(list, size);

    if (correct)
        fprintf(output, "No errors, program is syntactically correct.\n\n");
    else
    {
        symbolRepresent(list, err);
        fprintf(output, "    ***** ");
        error(err2);
        return 0;
    }

    //Writes intermediate code to file
    generateCode();

    //Runs intermediate code on virtual machine
    makeStack();

    fclose(input);
    fclose(output);
    return 0;
}
