#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//Macros
#define JSON_FILE_PATH "DATA/file.json"

//Color Macros
#ifdef __unix
#define BLEU_COLOR "\033[1;34m"
#define YELLOW_COLOR "\033[1;33m"
#define GREEN_COLOR "\033[1;32m"
#define RED_COLOR "\033[1;31m"
#define WHITE_COLOR "\033[00m"
#endif

#ifndef __unix
#define BLEU_COLOR ""
#define YELLOW_COLOR ""
#define GREEN_COLOR ""
#define RED_COLOR ""
#define WHITE_COLOR ""
#endif

//Data structurs
typedef enum {

    false,
    true

} bool;

typedef enum {

    null,
    string,
    integer,
    decimal,
    boolean,
    jsonList,
    jsonObj

} value_types;

typedef struct {

    value_types type;
    void *value;

} type_value;

typedef struct {

    char *key;
    type_value value;
    
} key_value;

typedef struct {

    type_value *value;
    unsigned long number_of_list_key_values;
    
} list;

typedef struct {
    key_value *json_key_values;
    unsigned long number_of_json_key_values;
} json;

typedef struct {
    char *number;
    bool isDecimal;
} parsed_number;

typedef struct {
    char *key;
    unsigned long numberOfValues;
} statistics_key_nbrValue;

//  Prototypes
json *get_json_parts(FILE *file);
list *get_list_parts(FILE *file);
char *get_string(FILE *file, char c);
parsed_number *get_number(FILE *file, char c);
void statistics_on_jsonObj(json * parsed_json_part);
void check_for_jsonObj(list * parsed_list_part);

//Printing prototypes
void add_tabs(const int n);
void print_json(const json *json_parsed, int level);
void print_list(const list *list_parsed, int level);

//Main Program
int main(void){
    
    FILE *file = fopen(JSON_FILE_PATH, "r");
    assert(file != NULL);

    json *json_parsed;
    
    json_parsed = (json*) malloc(sizeof(json));

    json_parsed = get_json_parts(file);

    print_json(json_parsed, 0);


    statistics_on_jsonObj(json_parsed);

    free(json_parsed);
    fclose(file);
    return 0;
}

//Functions
json *get_json_parts(FILE *file){
    char c;
    json *partial_json;

    partial_json = (json*) malloc(sizeof(json));
    partial_json->number_of_json_key_values = 0;
        c = fgetc(file);
        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
        do{
            if (c == '{')
            {
                do{
                    key_value *p;
                    partial_json->number_of_json_key_values++;

                    p = (key_value *) malloc(partial_json->number_of_json_key_values * sizeof(key_value));
                    if(p){
                        memcpy(p, partial_json->json_key_values, (partial_json->number_of_json_key_values-1) * sizeof(key_value));
                        partial_json->json_key_values = p;
                    }
                    else{
                        printf("reallocation faild\n");
                        assert(p == NULL);
                    }

                    c = fgetc(file);
                    while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    if(c == '}'){
                        partial_json->json_key_values[(partial_json->number_of_json_key_values)-1].key = NULL;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = null;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = NULL;
                        return partial_json;
                    }
                    assert(c != EOF);
                    assert(c == '\"');

                    c = fgetc(file);
                    assert(c != EOF);

                    partial_json->json_key_values[(partial_json->number_of_json_key_values)-1].key = (char*) malloc(sizeof(char));
                    strcpy(partial_json->json_key_values[(partial_json->number_of_json_key_values)-1].key, get_string(file, c));
                    c = fgetc(file);
                    while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    assert(c == ':');

                    c = fgetc(file);
                    while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    assert(c != EOF);
                    assert(c != ',');

                    
                    if( c == '\"'){
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = string;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (char*) malloc(sizeof(char));
                        
                        c = fgetc(file);
                        strcpy(partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value, get_string(file,c));
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                        assert(c != EOF);

                        if(c == '}'){
                            return partial_json;
                        }
                    }
                    else if( c == '{'){
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = jsonObj;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (json*) malloc(sizeof(json));
                        
                        fseek(file, -1 , SEEK_CUR);
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (json*) get_json_parts(file);
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    }
                    else if( c == '-' || (c >= '0' && c <= '9') ){
                        parsed_number *number = (parsed_number*) malloc(sizeof(parsed_number));

                        if(c == '-'){
                            c = fgetc(file);
                            assert((c >= '0' && c <= '9') || c == '.');
                            number = get_number(file, c);
                            if(number->isDecimal){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = decimal;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (double*) malloc(sizeof(double));
                                *((double*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value) = (double) atof(number->number) * -1.0f;
                            }
                            else{
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = integer;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (long*) malloc(sizeof(long));
                                *((long*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value) = (long) atoi(number->number) * -1;
                            }
                        }
                        else{
                            assert((c >= '0' && c <= '9') || c == '.');
                            number = get_number(file, c);
                            if(number->isDecimal){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = decimal;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (double*) malloc(sizeof(double));
                                *((double*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value) = (double) atof(number->number);
                            }
                            else{
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = integer;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (long*) malloc(sizeof(long));
                                *((long*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value) = (long) atoi(number->number);
                            }
                        }

                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                        assert(c != EOF);

                        if(c == '}'){
                            return partial_json;
                        } 
                    }
                    else if(c == 'n' || c == 't' || c == 'f'){
                        if(c == 'n' || c == 't'){
                            char word[4];
                            for(size_t i = 0 ; i < 4 ; i++){
                                assert(c == 'n' || c == 'u' || c == 'l' || c == 't' || c == 'r' || c == 'e');
                                word[i] = c;
                                c = fgetc(file); 
                            }
                            if(strncmp("null", word, 4) == 0){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = null;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = NULL;
                            }
                            else if(strncmp("true", word, 4) == 0){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = boolean;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (bool*) malloc(sizeof(bool));
                                *((bool*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value) = true;
                            }
                            else{
                                printf(".......File syntax error......\n");
                            }
                            
                        }
                        else if(c == 'f'){
                            char word[5];
                            for(size_t i = 0 ; i < 5 ; i++){
                                assert(c == 'f' || c == 'a' || c == 'l' || c == 's' || c == 'e');
                                word[i] = c;
                                c = fgetc(file); 
                            }
                            if(strncmp("false", word, 5) == 0){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = boolean;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (bool*) malloc(sizeof(bool));
                                *((bool*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value) = false;
                            }
                            else{
                                printf(".......File syntax error......\n");
                            }
                        }

                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                        assert(c != EOF);

                        if(c == '}'){
                            return partial_json;
                        } 
                    }
                    else if (c == '['){
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = jsonList;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (list*) malloc(sizeof(list));
                        
                        fseek(file, -1 , SEEK_CUR);
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (list*) get_list_parts(file);
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    }
                }while(c == ',');
            }
        }while(c == ',');
    return partial_json;
}

list *get_list_parts(FILE *file){
    char c;
    unsigned short char_counter = 0;
    list *partial_list;
    type_value *p;

    partial_list = (list*) malloc(sizeof(list));
    partial_list->number_of_list_key_values = 0;
        c = fgetc(file);
        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
        do{
            if (c == '[')
            {
                do{
                    partial_list->number_of_list_key_values++;
                    p = (type_value *) realloc(partial_list->value, partial_list->number_of_list_key_values * sizeof(type_value));
                    if(p){
                        partial_list->value = p;
                    }
                    else{
                        printf("reallocation faild\n");
                        assert(p == NULL);
                    }

                    c = fgetc(file);
                    while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    assert(c != EOF);
                    assert(c != ',');

                    if(c == ']'){
                        partial_list->value[partial_list->number_of_list_key_values-1].type = null;
                        partial_list->value[partial_list->number_of_list_key_values-1].value = NULL;
                        return partial_list;
                    }

                    if( c == '\"'){
                        partial_list->value[partial_list->number_of_list_key_values-1].type = string;
                        partial_list->value[partial_list->number_of_list_key_values-1].value = (char*) malloc(sizeof(char));
                        
                        c = fgetc(file);
                        strcpy(partial_list->value[partial_list->number_of_list_key_values-1].value, get_string(file,c));
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                        assert(c != EOF);

                        if(c == ']'){
                            return partial_list;
                        }
                    }
                    else if( c == '{'){
                        partial_list->value[partial_list->number_of_list_key_values-1].type = jsonObj;
                        partial_list->value[partial_list->number_of_list_key_values-1].value = (json*) malloc(sizeof(json));
                        
                        fseek(file, -1 , SEEK_CUR);
                         partial_list->value[partial_list->number_of_list_key_values-1].value = (json*) get_json_parts(file);
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    }
                    else if( c == '-' || (c >= '0' && c <= '9')){
                        parsed_number *number = (parsed_number*) malloc(sizeof(parsed_number));

                        if(c == '-'){
                            c = fgetc(file);
                            assert((c >= '0' && c <= '9') || c == '.');
                            number = get_number(file, c);
                            if(number->isDecimal){
                                 partial_list->value[partial_list->number_of_list_key_values-1].type = decimal;
                                 partial_list->value[partial_list->number_of_list_key_values-1].value = (double*) malloc(sizeof(double));
                                *((double*) partial_list->value[partial_list->number_of_list_key_values-1].value) = (double) atof(number->number) * -1.0f;
                            }
                            else{
                                 partial_list->value[partial_list->number_of_list_key_values-1].type = integer;
                                 partial_list->value[partial_list->number_of_list_key_values-1].value = (long*) malloc(sizeof(long));
                                *((long*) partial_list->value[partial_list->number_of_list_key_values-1].value) = (long) atoi(number->number) * -1;
                            }
                        }
                        else{
                            assert((c >= '0' && c <= '9') || c == '.');
                            number = get_number(file, c);
                            if(number->isDecimal){
                                partial_list->value[partial_list->number_of_list_key_values-1].type = decimal;
                                partial_list->value[partial_list->number_of_list_key_values-1].value = (double*) malloc(sizeof(double));
                                *((double*) partial_list->value[partial_list->number_of_list_key_values-1].value) = (double) atof(number->number);
                            }
                            else{
                                partial_list->value[partial_list->number_of_list_key_values-1].type = integer;
                                partial_list->value[partial_list->number_of_list_key_values-1].value = (long*) malloc(sizeof(long));
                                *((long*) partial_list->value[partial_list->number_of_list_key_values-1].value) = (long) atoi(number->number);
                            }
                        }

                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                        assert(c != EOF);

                        if(c == ']'){
                            return partial_list;
                        } 
                    }
                    else if(c == 'n' || c == 't' || c == 'f'){
                        if(c == 'n' || c == 't'){
                            char word[4];
                            for(size_t i = 0 ; i < 4 ; i++){
                                assert(c == 'n' || c == 'u' || c == 'l' || c == 't' || c == 'r' || c == 'e');
                                word[i] = c;
                                c = fgetc(file); 
                            }
                            if(strncmp("null", word, 4) == 0){
                                partial_list->value[partial_list->number_of_list_key_values-1].type = null;
                                partial_list->value[partial_list->number_of_list_key_values-1].value = NULL;
                            }
                            else if(strncmp("true", word, 4) == 0){
                                partial_list->value[partial_list->number_of_list_key_values-1].type = boolean;
                                partial_list->value[partial_list->number_of_list_key_values-1].value = (bool*) malloc(sizeof(bool));
                                *((bool*)partial_list->value[partial_list->number_of_list_key_values-1].value) = true;
                            }
                            else{
                                printf(".......File syntax error......\n");
                            }
                            
                        }
                        else if(c == 'f'){
                            char word[5];
                            for(size_t i = 0 ; i < 5 ; i++){
                                assert(c == 'f' || c == 'a' || c == 'l' || c == 's' || c == 'e');
                                word[i] = c;
                                c = fgetc(file); 
                            }
                            if(strncmp("false", word, 5) == 0){
                                partial_list->value[partial_list->number_of_list_key_values-1].type = boolean;
                                partial_list->value[partial_list->number_of_list_key_values-1].value = (bool*) malloc(sizeof(bool));
                                *((bool*)partial_list->value[partial_list->number_of_list_key_values-1].value) = false;
                            }
                            else{
                                printf(".......File syntax error......\n");
                            }
                        }

                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                        assert(c != EOF);

                        if(c == ']'){
                            return partial_list;
                        } 
                    }
                    else if (c == '['){
                        partial_list->value[partial_list->number_of_list_key_values-1].type = jsonList;
                        partial_list->value[partial_list->number_of_list_key_values-1].value = (list*) malloc(sizeof(list));
                        
                        fseek(file, -1 , SEEK_CUR);
                        partial_list->value[partial_list->number_of_list_key_values-1].value = (list*) get_list_parts(file);
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n' || c == '\r')c = fgetc(file);
                    }
                }while(c == ',');
            }
        }while(c == ',');
    return partial_list;
}

char *get_string(FILE *file, char c){
    unsigned long long char_counter = 0;
    char *string = NULL;
    while(c != '\"'){
        assert(c != EOF);
        char_counter++;

        //Add space for one more caracter on key
        string = (char *)realloc(string,char_counter*sizeof(char));
        string[char_counter-1] = c;
                
        c = fgetc(file);
    }
        
    //Add \0 caracter on key
    string = (char *)realloc(string,(char_counter+1)*sizeof(char));
    string[char_counter] = '\0';

    return string;
}

parsed_number *get_number(FILE *file, char c){
    unsigned long long char_counter = 0;
    parsed_number *number;

    number = (parsed_number*) malloc(sizeof(parsed_number));
    number->isDecimal=false;

    while((c >= '0' && c <= '9') || c == '.'){
        assert(c != EOF);
        char_counter++;
        if(c == '.')number->isDecimal=true;
        //Add space for one more caracter on key
        number->number = (char *)realloc(number->number,char_counter*sizeof(char));
        number->number[char_counter-1] = c;
                
        c = fgetc(file);
    }
    
    //Add \0 caracter on key
    number->number = (char *)realloc(number->number,(char_counter+1)*sizeof(char));
    number->number[char_counter] = '\0';
    fseek(file, -1, SEEK_CUR);

    return number;
}

//Procedures
void statistics_on_jsonObj(json * parsed_json_part){
    size_t i = 0;
    key_value * my_key_value = NULL;
    for(i = 0 ; i < parsed_json_part->number_of_json_key_values ; i++){
        my_key_value = &parsed_json_part->json_key_values[i];
        if(my_key_value->value.type == jsonObj){
            statistics_on_jsonObj(my_key_value->value.value);
        }
        else if(my_key_value->value.type == jsonList){
            list * myList = (list *)(my_key_value->value.value);
            check_for_jsonObj(myList);
            unsigned long number_of_elements = myList->number_of_list_key_values;
            printf("In \"%s\" there %s "GREEN_COLOR"%d"WHITE_COLOR" %s%c\n", my_key_value->key, number_of_elements > 1u?"are":"is", number_of_elements, my_key_value->key, number_of_elements > 1u?'s':'\0');
        }
    }
}

void check_for_jsonObj(list * parsed_list_part){
    size_t i = 0;
    type_value * my_value = &parsed_list_part->value[i];
    for(i = 0 ; i < parsed_list_part->number_of_list_key_values ; i++){
        if(my_value->type == jsonObj){
            statistics_on_jsonObj(my_value->value);
        }
        else if(my_value->type == jsonList){
            list * myList = (list *)(my_value->value);
            check_for_jsonObj(myList);
        }
    }
}


void add_tabs(const int n){
    for(size_t i = 0 ; i < n ; i++){
        putc('\t',stdout);
    }
}

void print_json(const json *json_parsed, int level){
    size_t i = 0;
    printf("{\n");
    for(i = 0 ; i < json_parsed->number_of_json_key_values ; i++){
        if(json_parsed->json_key_values[i].key == NULL){
            add_tabs(level);printf(RED_COLOR"null"WHITE_COLOR);
            continue;
        }
        switch(json_parsed->json_key_values[i].value.type){
            case null : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : "RED_COLOR"null"WHITE_COLOR, json_parsed->json_key_values[i].key);break;
            case string : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : "YELLOW_COLOR"\"%s\""WHITE_COLOR, json_parsed->json_key_values[i].key, json_parsed->json_key_values[i].value.value);break;
            case integer : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : "GREEN_COLOR"%d"WHITE_COLOR, json_parsed->json_key_values[i].key, *(long*)json_parsed->json_key_values[i].value.value);break;
            case decimal : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : "GREEN_COLOR"%f"WHITE_COLOR, json_parsed->json_key_values[i].key, *(double*)json_parsed->json_key_values[i].value.value);break;
            case boolean : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : %s", json_parsed->json_key_values[i].key, *((bool*)json_parsed->json_key_values[i].value.value)?GREEN_COLOR"true"WHITE_COLOR:RED_COLOR"false"WHITE_COLOR);break;
            case jsonList : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : ",json_parsed->json_key_values[i].key);print_list(json_parsed->json_key_values[i].value.value, level+1);break;
            case jsonObj : add_tabs(level+1);printf(BLEU_COLOR"\"%s\""WHITE_COLOR" : ",json_parsed->json_key_values[i].key);print_json(json_parsed->json_key_values[i].value.value, level+1);break;
            default : printf("......Printing error......");break;
        }
        if(i < json_parsed->number_of_json_key_values-1){
            puts(",");
        }
        else{
            puts("");
        }
    }
    add_tabs(level);
    printf("}");
}

void print_list(const list *list_parsed, int level){
    size_t i = 0;
    printf("[\n");
    for(i = 0 ; i < list_parsed->number_of_list_key_values ; i++){
        switch(list_parsed->value[i].type){
            case null : add_tabs(level+1);printf(RED_COLOR"null"WHITE_COLOR);break;
            case string : add_tabs(level+1);printf(YELLOW_COLOR"\"%s\""WHITE_COLOR, list_parsed->value[i].value);break;
            case integer : add_tabs(level+1);printf(GREEN_COLOR"%d"WHITE_COLOR, *(long*)list_parsed->value[i].value);break;
            case decimal : add_tabs(level+1);printf(GREEN_COLOR"%f"WHITE_COLOR, *(double*)list_parsed->value[i].value);break;
            case boolean : add_tabs(level+1);printf("%s", *((bool*)list_parsed->value[i].value)?GREEN_COLOR"true"WHITE_COLOR:RED_COLOR"false"WHITE_COLOR);break;
            case jsonList : add_tabs(level+1);print_list(list_parsed->value[i].value, level+1);break;
            case jsonObj : add_tabs(level+1);print_json(list_parsed->value[i].value, level+1);break;
            default : printf("......Printing error......");break;
        }
        if(i < list_parsed->number_of_list_key_values-1){
            puts(",");
        }
        else{
            puts("");
        }
    }
    add_tabs(level);
    printf("]");
}