#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//Macros
#define JSON_FILE_PATH "DATA/file.json"

//Data structurs
typedef enum bool {

    false,
    true

} bool;

typedef enum value_type {

    null,
    string,
    integer,
    decimal,
    boolean,
    list,
    jsonObj

} value_type;

typedef struct key_value {

    char *key;
    value_type type;
    void *value;
    
} key_value;

typedef struct json {
    key_value *json_key_values;
    unsigned long number_of_json_key_values;
} json;

typedef struct {
    char *number;
    bool isDecimal;
} parsed_number;

//Prototypes
json *get_json_parts(FILE *file);
char *get_string(FILE *file, char c);
void add_tabs(unsigned n);
void print_json(const json *json_parsed, unsigned level);
parsed_number *get_number(FILE *file, char c);

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
    unsigned short char_counter = 0;
    json *partial_json;
    key_value *p;

    partial_json = (json*) malloc(sizeof(json));
    partial_json->number_of_json_key_values = 0;
        c = fgetc(file);
        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
        do{
            if (c == '{')
            {
                do{
                    partial_json->number_of_json_key_values++;
                    p = (key_value *) realloc(partial_json->json_key_values, partial_json->number_of_json_key_values * sizeof(key_value));
                    if(p){
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
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = string;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (char*) malloc(sizeof(char));
                        
                        c = fgetc(file);
                        strcpy(partial_json->json_key_values[partial_json->number_of_json_key_values-1].value, get_string(file,c));
                        
                        c = fgetc(file);
                        while(c == ' ' || c == '\t' || c == '\n')c = fgetc(file);
                        assert(c != EOF);

                        if(c == '}'){
                            return partial_json;
                        }
                    }
                    else if( c == '{'){
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = jsonObj;
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (json*) malloc(sizeof(json));
                        
                        fseek(file, -1 , SEEK_CUR);
                        partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (json*) get_json_parts(file);
                        
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
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = decimal;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (double*) malloc(sizeof(double));
                                *((double*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value) = (double) atof(number->number) * -1.0f;
                            }
                            else{
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = integer;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (long*) malloc(sizeof(long));
                                *((long*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value) = (long) atoi(number->number) * -1;
                            }
                        }
                        else{
                            assert((c >= '0' && c <= '9') || c == '.');
                            number = get_number(file, c);
                            if(number->isDecimal){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = decimal;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (double*) malloc(sizeof(double));
                                *((double*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value) = (double) atof(number->number);
                            }
                            else{
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = integer;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (long*) malloc(sizeof(long));
                                *((long*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value) = (long) atoi(number->number);
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
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = null;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = NULL;
                            }
                            else if(strncmp("true", word, 4) == 0){
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = boolean;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (bool*) malloc(sizeof(bool));
                                *((bool*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value) = true;
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
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].type = boolean;
                                partial_json->json_key_values[partial_json->number_of_json_key_values-1].value = (bool*) malloc(sizeof(bool));
                                *((bool*)partial_json->json_key_values[partial_json->number_of_json_key_values-1].value) = false;
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
                    // else if (c == '['){

                    // }
                }while(c == ',');
            }
        }while(c == ',');
    return partial_json;
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
        switch(json_parsed->json_key_values[i].type){
            case null : add_tabs(level);printf("\t\"%s\" : null,\n", json_parsed->json_key_values[i].key);break;
            case string : add_tabs(level);printf("\t\"%s\" : \"%s\",\n", json_parsed->json_key_values[i].key, json_parsed->json_key_values[i].value);break;
            case integer : add_tabs(level);printf("\t\"%s\" : %d,\n", json_parsed->json_key_values[i].key, *(long*)json_parsed->json_key_values[i].value);break;
            case decimal : add_tabs(level);printf("\t\"%s\" : %f,\n", json_parsed->json_key_values[i].key, *(double*)json_parsed->json_key_values[i].value);break;
            case boolean : add_tabs(level);printf("\t\"%s\" : %s,\n", json_parsed->json_key_values[i].key, *((bool*)json_parsed->json_key_values[i].value)?"true":"false");break;
            case jsonObj : add_tabs(level);printf("\t\"%s\" : ",json_parsed->json_key_values[i].key);print_json(json_parsed->json_key_values[i].value, level+1);break;
            default : printf("......Printing error......");break;
        }
    }
    add_tabs(level);
    printf("},\n");
}