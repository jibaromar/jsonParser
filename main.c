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

//  Prototypes
json *get_json_parts(FILE *file);
list *get_list_parts(FILE *file);
char *get_string(FILE *file, char c);
parsed_number *get_number(FILE *file, char c);

//Printing prototypes
void add_tabs(unsigned n);
void print_json(const json *json_parsed, unsigned level);
void print_list(const list *list_parsed, unsigned level);

//Main Program
int main(void){
    
    FILE *file = fopen(JSON_FILE_PATH, "r");
    assert(file != NULL);

    json *json_parsed;
    char c = 0;
    
    json_parsed = (json*) malloc(sizeof(json));

    json_parsed = get_json_parts(file);

    print_json(json_parsed, 0);

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
        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                    while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
                    assert(c != EOF);
                    assert(c == '\"');

                    c = fgetc(file);
                    assert(c != EOF);

                    partial_json->json_key_values[(partial_json->number_of_json_key_values)-1].key = (char*) malloc(sizeof(char));
                    strcpy(partial_json->json_key_values[(partial_json->number_of_json_key_values)-1].key, get_string(file, c));
                    c = fgetc(file);
                    while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
                    assert(c == ':');

                    c = fgetc(file);
                    while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
                    assert(c != EOF);
                    assert(c != ',');

                    
                    if( c == '\"'){
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.type = string;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value = (char*) malloc(sizeof(char));
                        
                        c = fgetc(file);
                        strcpy(partial_json->json_key_values[partial_json->number_of_json_key_values-1].value.value, get_string(file,c));
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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

                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                    while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
                    assert(c != EOF);
                    assert(c != ',');

                    
                    if( c == '\"'){
                        partial_list->value[partial_list->number_of_list_key_values-1].type = string;
                        partial_list->value[partial_list->number_of_list_key_values-1].value = (char*) malloc(sizeof(char));
                        
                        c = fgetc(file);
                        strcpy(partial_list->value[partial_list->number_of_list_key_values-1].value, get_string(file,c));
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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

                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
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
void add_tabs(unsigned n){
    for(size_t i = 0 ; i < n ; i++){
        putc('\t',stdout);
    }
}

void print_json(const json *json_parsed, unsigned level){
    size_t i = 0;
    //add_tabs(level);
    printf("{\n");
    for(i = 0 ; i < json_parsed->number_of_json_key_values ; i++){
        switch(json_parsed->json_key_values[i].value.type){
            case null : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : "RED_COLOR"null"WHITE_COLOR",\n", json_parsed->json_key_values[i].key);break;
            case string : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : "YELLOW_COLOR"\"%s\""WHITE_COLOR",\n", json_parsed->json_key_values[i].key, json_parsed->json_key_values[i].value.value);break;
            case integer : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : "GREEN_COLOR"%d"WHITE_COLOR",\n", json_parsed->json_key_values[i].key, *(long*)json_parsed->json_key_values[i].value.value);break;
            case decimal : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : "GREEN_COLOR"%f"WHITE_COLOR",\n", json_parsed->json_key_values[i].key, *(double*)json_parsed->json_key_values[i].value.value);break;
            case boolean : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : \033[0;31m%s\033[00m,\n", json_parsed->json_key_values[i].key, *((bool*)json_parsed->json_key_values[i].value.value)?GREEN_COLOR"true":RED_COLOR"false");break;
            case jsonList : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : ",json_parsed->json_key_values[i].key);print_list(json_parsed->json_key_values[i].value.value, level+1);break;
            case jsonObj : add_tabs(level);printf("\t"BLEU_COLOR"\"%s\""WHITE_COLOR" : ",json_parsed->json_key_values[i].key);print_json(json_parsed->json_key_values[i].value.value, level+1);break;
            default : printf("......Printing error......");break;
        }
    }
    add_tabs(level);
    printf("},\n");
}

void print_list(const list *list_parsed, unsigned level){
    size_t i = 0;
    //add_tabs(level);
    printf("[\n");
    for(i = 0 ; i < list_parsed->number_of_list_key_values ; i++){
        switch(list_parsed->value[i].type){
            case null : add_tabs(level);printf("\t"RED_COLOR"null"WHITE_COLOR",\n");break;
            case string : add_tabs(level);printf("\t"YELLOW_COLOR"\"%s\""WHITE_COLOR",\n", list_parsed->value[i].value);break;
            case integer : add_tabs(level);printf("\t"GREEN_COLOR"%d"WHITE_COLOR",\n", *(long*)list_parsed->value[i].value);break;
            case decimal : add_tabs(level);printf("\t"GREEN_COLOR"%f"WHITE_COLOR",\n", *(double*)list_parsed->value[i].value);break;
            case boolean : add_tabs(level);printf("\t%s,\n", *((bool*)list_parsed->value[i].value)?GREEN_COLOR"true":RED_COLOR"false");break;
            case jsonList : add_tabs(level);print_list(list_parsed->value[i].value, level+1);break;
            case jsonObj : add_tabs(level);print_json(list_parsed->value[i].value, level+1);break;
            default : printf("......Printing error......");break;
        }
    }
    add_tabs(level);
    printf("],\n");
}