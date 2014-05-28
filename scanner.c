#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFSIZE 4096

char* IDEN = "IDEN"; // ( [a-z] | [A-Z] | _ ) ([0-9] | [a-z] | [A-Z] | _)* 
char* REWD = "REWD"; // if, else, while, for, do, switch, case, int, float, double, char
char* INTE = "INTE"; // integer number
char* FLOT = "FLOT"; // float number ->3.5E+5, 3.5e5, 3.5e-5, 3.5e50
char* OPER = "OPER"; // +, -, *, /,=
char* SPEC = "SPEC"; // {, }, (, ), ;, ',,, :
//				comment -> // ...............
char* RESERVE[] = {"if","else","while","for","do","switch","case","int","float","double","char","struct","const","break","return","continue"}; 
/* --Bonus-- */
const char* PRE = "PRE";

typedef struct Datum{
	int line;
	char* type;
	char* value;
} TOKEN;

typedef struct Node{
	TOKEN* element;
	struct Node* next;
} node;

TOKEN* newToken(int num, char* type, char* val);
node* createNode(TOKEN* obj);
node* insertNode(node* header, node* current);
node* printNode(node* header);
node* writeNode(node* header,FILE* fp);

int ignore(char dest,FILE* fp);
int* keyword(FILE* fp,char target[]);
int isReservedWord(char word[]);


int main(int argc, char* argv[]){
	if(argc>1){
		FILE *source = NULL;
		printf("Open File: %s\n",argv[1]);
		source = fopen(argv[1],"r");
		/*  check source code exists */
		if(source == NULL){
			printf("No such file...Failed\n");
			exit(1);
		}
		else{
			int lnum = 1, textIdx=0, textLen=0, flag=0;
			char buf[BUFFSIZE];
			char c;
			char numType; /* State: 0->int ; 1->float ; 2->float+(e|E|) */
			node* head;
			node* current;	
			head = current = NULL;
			printf("Success!!\n");
			/*  Single Scanner (Not yet been Structured) */
			c = getc(source);
			while(c != EOF){
				switch(c){
					// filter out String
					case '"':
						flag = ignore('"',source);
						if(flag == EOF)
							printf("Line: %d Error: File Ending Before String Reading\n",lnum);
						else
							lnum += flag; //record lines for case : "aaa~~~~~ \n ~~~~bbb"
						printf("Line: %d @String Ignored@\n",lnum);
						flag &= 0;
					break;
					// filter out preprocessor
					case '#':
						while(c != '\n')
							c = getc(source);
						printf("Line: %d @Preprocessor Ignored@\n",lnum); /* Ignored EOF case  */
						lnum++;
					break;

					case '/':
						c=getc(source); /* Peek one character */
						flag = 1;
						switch(c){
							case '/':
								while(c != '\n')
									c = getc(source);
								printf("Line: %d @Comment Ignored@\n",lnum);
								lnum++;	
							break;

							case '*':
								printf("Line: %d @!Comment\n",lnum);
								do{
									c = getc(source);
									lnum = (c =='\n')?lnum+1:lnum;
									if(c == '*'){
										c = getc(source); // peek 1 character
										if(c == '/'){
											flag = 0;
										}
										else{
											fseek(source,-1,SEEK_CUR);
										}
									}

								}while( flag == 1 ); /* Ignored EOF case  */
								printf("Line: %d Comment!@\n",lnum);
							break;

							default:
								fseek(source,-1,SEEK_CUR); /* Move back one character */
								/* New Token */
								current = createNode( newToken(lnum,OPER,"/") );
								if(( head = insertNode( head, current ) )== NULL){
									printf("Insert Token Error-> Return NULL\n");
								}
							break;
						}
					break;
					/* Operand */
					case '+':
					case '-':
					case '*':
					case '=':
					case '>':
					case '<':
					case '!':
					case '|':
					case '&':
					case '^':
					case '~':
					case '%':
						buf[textIdx]=c;
						textLen++;
						do{
							char* operand;
							c = getc(source); /* Peek 1 Character */

							if(c == EOF)
							{
								printf("Line: %d Unexpected EOF\n",lnum);
								exit(1);
							}

							flag = 1;

							switch(c){
								case '+':
								case '-':
								case '*':
								case '=':
								case '>':
								case '<':
								case '!':
								case '|':
								case '&':
								case '^':
								case '~':
								case '%':
									buf[textIdx+textLen]=c;
									textLen++;
								break;

								default:
									fseek(source,-1,SEEK_CUR);
									operand = (char *)malloc(textLen+1); /* alloc enough memory of string dynamically */
									memset(operand,'\0',textLen+1);
									strncpy(operand,buf,textLen);
									/* New Token */
									current = createNode( newToken(lnum,OPER,operand) );
									if(( head = insertNode( head, current ) )== NULL){
										printf("Insert Token Error-> Return NULL\n");
									}
									//printf("%s\n",operand);
									textLen = 0;
									flag = 0;
								break;
							}
						}while(c != EOF && flag == 1);
					break;
					/* Number */
					case '.':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						buf[textIdx]=c;
						textLen++;
						flag = 1;
						numType = (c=='.')?1:0; /* State: 0->int ; 1->float ; 2->float+(e|E|) */

						do{
							char* number;
							c = getc(source); /* Peek 1 Character */

							if(c == EOF)
							{
								printf("Line: %d Unexpected EOF\n",lnum);
								exit(1);
							}

							switch(c){
								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
								case '8':
								case '9':
									buf[textIdx+textLen]=c;
									textLen++;
								break;
								/* Any case 3.01.456 or 3.5e04 */
								case '.':
									if(numType > 0){
										fseek(source,-1,SEEK_CUR);
										number = (char *)malloc(textLen+1); /* alloc enough memory of string dynamically */
										memset(number,'\0',textLen+1);
										strncpy(number,buf,textLen);
										/* New Token */
										current = createNode( newToken(lnum,FLOT,number) );
										if(( head = insertNode( head, current ) )== NULL){
											printf("Insert Token Error-> Return NULL\n");
										}
										textLen = 0;
										flag = 0;
									}
									else{
										numType = 1;
										buf[textIdx+textLen]=c;
										textLen++;
									}
								break;
								/* Any case 3.0e or 789e04 */
								case 'e':
								case 'E':
									if(numType == 1){
										numType++;
										buf[textIdx+textLen]=c;
										textLen++;
									}
									else{
										fseek(source,-1,SEEK_CUR);
										number = (char *)malloc(textLen+1); /* alloc enough memory of string dynamically */
										memset(number,'\0',textLen+1);
										strncpy(number,buf,textLen);
										/* New Token */
										if(numType == 0)
											current = createNode( newToken(lnum,INTE,number) );
										else
											current = createNode( newToken(lnum,FLOT,number) );
										if(( head = insertNode( head, current ) )== NULL){
											printf("Insert Token Error-> Return NULL\n");
										}
										textLen = 0;
										flag = 0;
									}
								break;
								/* Any case 30+ or 3.8e- or 7.+ */
								case '+':
								case '-':
									if(numType == 2){
										numType++;
										buf[textIdx+textLen]=c;
										textLen++;
									}
									else{
										fseek(source,-1,SEEK_CUR);
										number = (char *)malloc(textLen+1); /* alloc enough memory of string dynamically */
										memset(number,'\0',textLen+1);
										strncpy(number,buf,textLen);
										/* New Token */
										if(numType == 0)
											current = createNode( newToken(lnum,INTE,number) );
										else
											current = createNode( newToken(lnum,FLOT,number) );
										if(( head = insertNode( head, current ) )== NULL){
											printf("Insert Token Error-> Return NULL\n");
										}
										textLen = 0;
										flag = 0;
									}
								break;

								default:
									fseek(source,-1,SEEK_CUR);
									number = (char *)malloc(textLen+1); /* alloc enough memory of string dynamically */
									memset(number,'\0',textLen+1);
									strncpy(number,buf,textLen);
									/* New Token */
									if(numType == 0)
										current = createNode( newToken(lnum,INTE,number) );
									else
										current = createNode( newToken(lnum,FLOT,number) );
									if(( head = insertNode( head, current ) )== NULL){
										printf("Insert Token Error-> Return NULL\n");
									}
									textLen = 0;
									flag = 0;
								break;
							}
						}while(flag == 1 & c != EOF);
						numType = 0; //reset numType
					break;

					/* Case: '''' | '\\' (character Excluded) Special character */
					case '\'':
					case '{':
					case '}':
					case ':':
					case ';':
					case ',':
					case '(':
					case ')':
						/* New Token */
						{
							char* temp = malloc(sizeof(c)+1);
							memset(temp,'\0',sizeof(c)+1);
							temp[0] = c;
							current = createNode( newToken(lnum,SPEC,temp) );
							if(( head = insertNode( head, current ) )== NULL){
							printf("Insert Token Error-> Return NULL\n");
							}
						}
					break;

					case '\n':
						lnum++;
						//	printf("%c",c);
					break;
					/* Any Case ( _|[a-z]|[A-Z] )( _|[a-z]|[A-Z][0-9] )  */
					case '_':
					case 'a':
					case 'b':
					case 'c':
					case 'd':
					case 'e':
					case 'f':
					case 'g':
					case 'h':
					case 'i':
					case 'j':
					case 'k':
					case 'l':
					case 'm':
					case 'n':
					case 'o':
					case 'p':
					case 'q':
					case 'r':
					case 's':
					case 't':
					case 'u':
					case 'v':
					case 'w':
					case 'x':
					case 'y':
					case 'z':
					case 'A':
					case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
					case 'G':
					case 'H':
					case 'I':
					case 'J':
					case 'K':
					case 'L':
					case 'M':
					case 'N':
					case 'O':
					case 'P':
					case 'Q':
					case 'R':
					case 'S':
					case 'T':
					case 'U':
					case 'V':
					case 'W':
					case 'X':
					case 'Y':
					case 'Z':
						{
							char* word;
							int* wordType;
							fseek(source,-1,SEEK_CUR); // move back 1 character
							wordType = keyword(source,buf);
							word = (char*)malloc(wordType[1]+1);
							memset(word,'\0',wordType[1]+1);
							strncpy(word,buf,wordType[1]);

							if(wordType[0] == 0){
								current = createNode( newToken(lnum,IDEN,word) );
							}
							else{
								current = createNode( newToken(lnum,REWD,word) );
							}

							if(( head = insertNode( head, current ) )== NULL){
								printf("Insert Token Error-> Return NULL\n");
							}
						}
					break;

					default :
						if(c == '\t')
							printf("Unknown Character: %s\n","\\t");
						else
							printf("Unknown Character: %c\n",c);
					break;
				}
				c = getc(source);
			}
			if(printNode(head) == NULL){
				printf("print nodes Error -> Return NULL\n");
			}
			{
				FILE* output = fopen("tokens.txt","w");
				printf("Output File: %s\n","tokens.txt");
				if(writeNode(head,output) == NULL){
					printf("write nodes Error -> Return NULL\n");
				}
				else{
					printf("Done. Success!! \n");
				}
			}
		}

	}
	return 0;
}

TOKEN* newToken(int num, char* type, char* val){
	TOKEN* _token = (TOKEN*)malloc(sizeof(TOKEN));
	_token->line = num;
	_token->type = type;
	_token->value = val;
	return _token;
}

node* createNode(TOKEN* obj){
	node* _node = (node*)malloc(sizeof(node));
	_node->element = obj;
	_node->next = NULL;
	return _node;
}

node* insertNode(node* header, node* current){
	if(header == NULL){
		TOKEN* t;
		header = current;
		t = header->element;
		current = current->next;
	}
	else if(current == NULL){
		printf("Current Node Error\n");
	}
	else{
		node* pos;
		for(pos = header;pos->next!=NULL;pos=pos->next);
		pos->next = current;
		current = current->next;
	}
	return header;
}

node* printNode(node* header){
	if(header == NULL)
		return header;
	else{
		node* n;
		for(n=header;n!=NULL;n=n->next){
			TOKEN* t = n->element;
			printf("%d\\%s\\%s\n",t->line,t->type,t->value);
		}
		return header;
	}
}

node* writeNode(node* header, FILE* fp){
	if(header == NULL)
		return header;
	else{
		node* n;
		for(n=header;n!=NULL;n=n->next){
			TOKEN* t = n->element;
			fprintf(fp,"%d\\%s\\%s\n",t->line,t->type,t->value);
		}
		return header;
	}
}

/* ignore characters until dest */
int ignore(char dest,FILE* fp){
	char c = getc(fp);
	int lines = 0;
	while(c != dest)
	{
		c = getc(fp);
		switch(c){
			case EOF:
				return EOF;
			break;
			case '\n':
				lines++;
			break;
			default:
				continue;
			break;
		}
	}
	return lines;
}
int isReservedWord(char word[]){
	int index=0, length = sizeof(RESERVE) / sizeof(*RESERVE), wordType=0;
	for(;index<length;index++){
		if(strcmp(word,RESERVE[index]) == 0){
			wordType = 1;
			return wordType;
		}
	}
	return wordType;
}
/* return keyword type-> 0 : IDEN ; 1 : REWD 
 * first character match ([a-z]) | ([A-Z]) | (_)
*/
int* keyword(FILE* fp,char target[]){
	char c;
	int* result = (int*)malloc(sizeof(int)*2);
	int tIndex = 0, tLen = 0, flag = 1, state=0;
	memset(result,0,sizeof(int)*2);

	while(flag == 1){
		c = getc(fp);

		if(c == EOF){
			printf("In Function Keyword: Unexpected EOF\n");
			exit(1);
		}
		switch(c){

			case '_':
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				target[tIndex+tLen]=c;
				tLen++;
			break;

			default:
				fseek(fp,-1,SEEK_CUR);
				flag = 0;
			break;
		}
	}
	if(tLen == 0){
		printf("Unexpected Keyword\n");
		exit(1);
	}
	else{
		char* temp = (char*) malloc(tLen+1);
		memset(temp,'\0',tLen+1);
		strncpy(temp, target, tLen);
		/* check is it a reservedWord ? */
		result[0] = isReservedWord(temp);
		result[1] = tLen;
	}
	return result;
}



