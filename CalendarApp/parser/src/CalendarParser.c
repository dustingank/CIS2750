/*
Name: Yizhou Wang
ID:1013411
*/
// this is parser file

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include "LinkedListAPI.h"
#include "CalendarParser.h"

// declear for the helper functions
void event_object(Event **node_ref, char*);
bool isProporty(char *);
void make_property(Property **node_ref, char*);
ICalErrorCode check_error(char *[], int);
void flod_line(const char*, FILE*);
bool check_the_rest(Property *);
bool check_alarm_prop(Property *);
char *alarmListToJSON( List *alarm_list);
char *alarmToJSON( Alarm *alarm);
char *property_json(Event*);
char *glue_code_getOtherInfo(char *, int );
char *make_calendar_file(char *, char *, char *, char *,char *, char *, char*, char*, char*, char*);
char *add_event_toFile(char*, char*, char*, char*, char*, char*, char*, char*, char*);


ListIterator tempFunction(const List*);

/*Function to create a Calendar object based on the contents of an iCalendar file.*/
ICalErrorCode createCalendar(char *fileName, Calendar** obj) {
    FILE *fptr = NULL;
    Property *object_property = NULL, *new_node_property= NULL, *alarm_property = NULL;
    Alarm *alarm;
    Event *new_node_event = NULL;
    List *ObjPropertyList = NULL;
    List *EventList = NULL;
    List *EvePropertyList = NULL;
    List *AlarmList = NULL;
    List *alarm_property_list = NULL;
    char *file_info[1024], string[1024], c, file_Name[200];
    bool  isevent = false, isalarm = false;
    int lines = 0;
   
    *obj = (Calendar *)malloc (sizeof(Calendar)); // allocate memory sapce for Calender Object

    //alarm_property_list = initializeList(&printProperty, &deleteProperty, &compareProperties);
    EvePropertyList = initializeList(&printProperty, &deleteProperty, &compareProperties);
    ObjPropertyList = initializeList(&printProperty, &deleteProperty, &compareProperties);
    EventList = initializeList(&printEvent, &deleteEvent, &compareEvents);
    AlarmList = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);

    strcpy(file_Name, "./uploads/");
    strcat(file_Name, fileName);
    fptr = fopen(file_Name, "r");
  
    if (fptr == NULL){ // error message
        free(EvePropertyList);
        free(ObjPropertyList);
        free(EventList);
        free(AlarmList);
        free(*obj);
        return INV_FILE;
    }

    if(strcmp(strchr(fileName, '.'), ".ics")){
        free(EvePropertyList);
        free(ObjPropertyList);
        free(EventList);
        free(AlarmList);
        free(*obj);
        fclose(fptr);
        return INV_FILE;
    }
    int count = 0, mark = 0;
    lines = 0;

    for (c = getc(fptr); c != EOF; c = getc(fptr)) {
        if (c == '\n') { 
            count++;
        } 
    }
    
    fseek(fptr, 0, SEEK_SET);

    while(!feof(fptr)) {
        int size_temp = 0;
        int length = 0;
        fgets(string, 1024, fptr);

        length = strlen(string);

        if(length > 0) {
            if(strstr(string, "\r") == NULL && mark < count) {
                free(EvePropertyList);
                free(ObjPropertyList);
                free(EventList);
                free(AlarmList);
                free(*obj);
                fclose(fptr);
                for(int line = 0; line < lines - 1; line++) {
                    if(file_info[line] != NULL) {
                        free(file_info[line]);
                    }
                }
                return INV_FILE;
            }
        }
        string[strcspn(string, "\r\n")] = 0;
        if(string[0] == ' ' || string[0] == '\t') {
            size_temp = strlen(string);
            memmove(string, string + 1, size_temp);
            file_info[lines-1] = realloc(file_info[lines-1], (sizeof(char) * (strlen(file_info[lines-1])) + 
                                                             sizeof(char) *(strlen(string) + 10)));
            strcat(file_info[lines-1], string);
        } else {
            if(string[0] != ';') {
                file_info[lines] = (char*)malloc(sizeof(char) * (strlen(string) + 10));
                strcpy(file_info[lines], string);
                lines++;
            }
        }
        mark++;
    }
   /* for(int line = 0; line < lines - 1; line++) {
        printf("------>%s\n", file_info[line]);
    } */

    if(check_error(file_info, lines) != 0){
        ICalErrorCode toReturn;
        toReturn = check_error(file_info, lines);
        free(EvePropertyList);
        free(ObjPropertyList);
        free(EventList);
        free(AlarmList);
        for(int a = 0; a < lines; a++) {
            free(file_info[a]);
        }
        free(*obj);
        fclose(fptr);
        return toReturn;
    }

    for(int line = 0; line < lines - 1; line++) {
        if (strstr(file_info[line], "VERSION") != NULL) { // get Version Float Start here;
            char *token = NULL;
            float version = 0.0;
            if(strcspn(file_info[line], ":") > strcspn(file_info[line], ";")) {
                token = strtok(file_info[line],";");
                token = strtok(NULL,"\0");
                version = atof(token);
                (*obj)->version = version;
            } else {
                token = strtok(file_info[line],":");
                token = strtok(NULL,"\0");
                version = atof(token);
                (*obj)->version = version;
            }
        } // get Version Float End here;

        if(strstr(file_info[line],"PRODID") != NULL) { // get PRODID Start here
            char *token = NULL;
            if(strcspn(file_info[line], ":") > strcspn(file_info[line], ";")) {
                token = strtok(file_info[line],";");
                token = strtok(NULL,"\0");
                strcpy((*obj)->prodID, token);
            } else {
                token = strtok(file_info[line],":");
                token = strtok(NULL,"\0");
                strcpy((*obj)->prodID, token);
            }
        } // get PRODID End here
        
        if(strstr(file_info[line],"BEGIN:VEVENT") != NULL) {
                new_node_event = (Event*) malloc (sizeof(Event));
                if(AlarmList == NULL) {
                    AlarmList = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
                }
                if(EvePropertyList == NULL) {
                    EvePropertyList = initializeList(&printProperty, &deleteProperty, &compareProperties);
                }
                isevent = true;
            }

        
        if(strstr(file_info[line],"END:VEVENT") != NULL) { // making the event list
            isevent = false;
            new_node_event->properties = EvePropertyList;
            new_node_event->alarms = AlarmList;
            EvePropertyList = NULL;
            AlarmList = NULL;
            insertBack(EventList, new_node_event);
        }
        
        if(isevent){ // making the event object
            if(strstr(file_info[line],"BEGIN:VALARM") != NULL) {
                isalarm = true;
                if(alarm_property_list == NULL) {
                    alarm_property_list = initializeList(&printProperty, &deleteProperty, &compareProperties);
                }
                alarm = (Alarm*)malloc(sizeof(Alarm));
            }

            if(strstr(file_info[line],"END:VALARM") != NULL) {
                alarm->properties = alarm_property_list;
                alarm_property_list = NULL;
                insertBack(AlarmList, alarm);
                isalarm = false;
            }

            if(isalarm) {   //making alarm
               
                if(strstr(file_info[line], "ACTION") != NULL) {
                    char *token;
                    if(strcspn(file_info[line], ":") > strcspn(file_info[line], ";")) {
                        token = strtok(file_info[line], ";");
                        token = strtok(NULL, "\0");
                        strcpy(alarm->action, token);
                    } else {
                        token = strtok(file_info[line], ":");
                        token = strtok(NULL, "\0");
                        strcpy(alarm->action, token);
                    }
                }
                
                if(strstr(file_info[line],"TRIGGER") != NULL) {
                    char *token;
                    if(strcspn(file_info[line], ":") > strcspn(file_info[line], ";")) {
                        token = strtok(file_info[line], ";");
                        token = strtok(NULL, "\0");
                        alarm->trigger = malloc(strlen(token) + 3);
                        strcpy(alarm->trigger, token);

                    } else {
                        token = strtok(file_info[line], ":");
                        token = strtok(NULL, "\0");
                        alarm->trigger = malloc(strlen(token) + 1);
                        strcpy(alarm->trigger, token);
                    }
                }

                if(isProporty(file_info[line])) {
                    make_property(&alarm_property, file_info[line]);
                    insertBack(alarm_property_list, alarm_property);
                }

            } // end of alarm

        if(isProporty(file_info[line]) && !isalarm){ // making properties for the events
            make_property(&new_node_property, file_info[line]);
            insertBack(EvePropertyList, new_node_property); 
        } else if (strstr(file_info[line], "STATUS") && !isalarm) {
            make_property(&new_node_property, file_info[line]);
            insertBack(EvePropertyList, new_node_property);
        }
        
        event_object(&new_node_event, file_info[line]);
        } // end of event object

        if(isProporty(file_info[line]) && !isevent &&!isalarm){
           
            make_property(&object_property, file_info[line]);
            insertBack(ObjPropertyList, object_property);

        }
        free(file_info[line]);
    }
    free(file_info[lines - 1]);
    
    (*obj)->properties = ObjPropertyList;
    (*obj)->events = EventList;
    fclose(fptr);
    return OK;
}
/*End*/
char *printError(ICalErrorCode err) {
    switch (err) {
        case OK:
            return "{\"Error_code\":\"OK\"}";
        case INV_FILE:
            return "{\"Error_code\":\"INV_FILE\"}";
        case INV_CAL:
            return "{\"Error_code\":\"INV_CAL\"}";
        case INV_VER:
            return "{\"Error_code\":\"INV_VER\"}";
        case DUP_VER:
            return "{\"Error_code\":\"DUP_VER\"}";
        case INV_PRODID:
            return "{\"Error_code\":\"INV_PRODID\"}";
        case DUP_PRODID:
            return "{\"Error_code\":\"DUP_PRODID\"}";
        case INV_EVENT:
            return "{\"Error_code\":\"INV_EVENT\"}";
        case INV_DT:
            return "{\"Error_code\":\"INV_DT\"}";
        case INV_ALARM:
            return "{\"Error_code\":\"INV_ALARM\"}";
        case OTHER_ERROR:
            return "{\"Error_code\":\"OTHER_ERROR\"}";
        case WRITE_ERROR:
            return "{\"Error_code\":\"WRITE_ERROR\"}";
        default:
            return "{\"Error_code\":\"Unkown\"}";
    }

    return OK;
}

ICalErrorCode writeCalendar(char *filename, const Calendar *obj) {
    FILE *fptr;
    ListIterator Event_ite, Property_Cal,pro_itr, alarm_itr;
    int Property_Cal_length = 0, Event_length = 0, Event_property_length = 0,
        alarm_length = 0;
    Property *temp_property;
    Alarm *temp_alarm;
    DateTime temp_data_time;
    Event *temp_event;

    // do something when obj == NULL

    if (obj == NULL || filename == NULL) {
        return WRITE_ERROR;
    }
    Event_ite = createIterator(obj->events);
    Event_length = getLength(obj->events);
    Property_Cal = createIterator(obj->properties);
    Property_Cal_length = getLength(obj->properties);

    fptr = fopen(filename, "w+");


    fputs("BEGIN:VCALENDAR\r\n",fptr);
    fprintf(fptr,"VERSION:%.1f\r\n", obj->version);
    fprintf(fptr,"PRODID:%s\r\n",obj->prodID);

    for (int a = 0; a < Property_Cal_length; a++) {
        temp_property = (Property*)nextElement(&Property_Cal);
        fprintf(fptr, "%s:%s\r\n", temp_property->propName, temp_property->propDescr);
    }

    while (Event_length > 0) {
        fputs("BEGIN:VEVENT\r\n", fptr);
        temp_event = (Event*)nextElement(&Event_ite);
        fprintf(fptr, "UID:%s\r\n", temp_event->UID);
        temp_data_time = (DateTime)(temp_event->creationDateTime);
        (temp_data_time.UTC) ? fprintf(fptr, "DTSTAMP:%sT%sZ\r\n", temp_data_time.date, temp_data_time.time) : fprintf(fptr, "DTSTAMP:%sT%s\r\n", temp_data_time.date, temp_data_time.time);
        
        temp_data_time = (DateTime)(temp_event->startDateTime);
        (temp_data_time.UTC) ? fprintf(fptr, "DTSTART:%sT%sZ\r\n", temp_data_time.date, temp_data_time.time) : fprintf(fptr, "DTSTART:%sT%s\r\n", temp_data_time.date, temp_data_time.time);

        pro_itr = createIterator(temp_event->properties);
        Event_property_length = getLength(temp_event->properties);

        while (Event_property_length > 0) {
            temp_property = (Property*)nextElement(&pro_itr);
            fprintf(fptr, "%s:%s\r\n", temp_property->propName, temp_property->propDescr);
            Event_property_length--;
        }

        alarm_itr = createIterator (temp_event->alarms);
        alarm_length = getLength(temp_event->alarms);

        while (alarm_length > 0) {
            temp_alarm = (Alarm*)nextElement(&alarm_itr);
            fputs("BEGIN:VALARM\r\n", fptr);
            fprintf(fptr, "ACTION:%s\r\nTRIGGER:%s\r\n", temp_alarm->action, temp_alarm->trigger);
            pro_itr = createIterator (temp_alarm->properties);

            for (int b = 0; b < getLength(temp_alarm->properties); b++) {
                temp_property = (Property*)nextElement(&pro_itr);
                fprintf(fptr, "%s:%s\r\n", temp_property->propDescr, temp_property->propName);
            }
            fputs("END:VALARM\r\n", fptr);
            alarm_length--;
        }
        fputs("END:VEVENT\r\n", fptr);
        Event_length--;
    }

    fputs("END:VCALENDAR\r\n", fptr);
    fclose(fptr);
    return OK;
}

void flod_line(const char *temp, FILE* fptr_temp) {
    int char_length = 0, num_fold = 0;
    char_length = strlen(temp);
    num_fold = char_length / 75;
    if(char_length % 75 != 0) {
        num_fold++;
    }
    for(int a = 0; a < num_fold; a++) {
        for(int b = a * 75; b < 75 * (a + 1); b++) {
            if(b > char_length - 1) {
                break;
            } else {
                fprintf(fptr_temp, "%c", temp[b]);
            }
        }
        if(num_fold - a > 1) {
            fputs("\r\n\t", fptr_temp);
        }
    }
    
}

ICalErrorCode validateCalendar(const Calendar* obj) {
    if(obj == NULL) {
        return INV_CAL;
    }

    if(obj->properties == NULL || obj->events == NULL) {
        return INV_CAL;
    } else if (getLength(obj->events) == 0) {
        return INV_CAL;
    }

    if(obj->prodID == NULL) {
        return INV_CAL;
    } else if (strlen(obj->prodID) == 0) {
       return INV_CAL;
    } else if(strlen(obj->prodID) > 1000) {
        return INV_CAL;
    }

    if(getLength(obj->properties) != 0) {
        if(getLength(obj->properties) > 2) {
            return INV_CAL;
        }
        int count = getLength(obj->properties);
        ListIterator pro_iterator = createIterator(obj->properties);
        Property *temp;
        while (count > 0) {
            temp = (Property*)nextElement(&pro_iterator);

            if(strlen(temp->propName) == 0) {
                return INV_CAL;
            }else if(strcmp(temp->propName, "CALSCALE") != 0 && strcmp(temp->propName,"METHOD") != 0) {
                return INV_CAL;
            } else if (strlen(temp->propDescr) == 0) {
                return INV_CAL;
            }
            count--;
        }
    }
    // end of testing the calendar part
    // start testing the event part
    int count = getLength(obj->events);
    ListIterator event_iterator = createIterator(obj->events);
    Event *temp;
    while(count > 0) {
        temp = (Event*)nextElement(&event_iterator);
        if(temp->properties == NULL || temp->alarms == NULL) {
            //printf("here1\n");
            return INV_EVENT;
        }

        if(strlen(temp->UID) == 0 || strlen(temp->UID) > 1000) {
            //printf("here2\n");
            return INV_EVENT;
        } 

        if(strlen(temp->creationDateTime.date) != 8) {
            //printf("here3\n");
            return INV_EVENT;
        } else if (strlen(temp->creationDateTime.time) != 6) {
            //printf("here4\n");
            return INV_EVENT;
        }
        if(strlen(temp->startDateTime.date) != 8) {
            //printf("here5\n");
            return INV_EVENT;
        } else if (strlen(temp->startDateTime.time) != 6) {
            //printf("here6\n");
            return INV_EVENT;
        }

        if(getLength(temp->properties) != 0) {
            int count = getLength(temp->properties);
            ListIterator event_prop_iterator = createIterator(temp->properties);
            Property *property_temp;
            bool class = false, created = false, description = false, geo = false,
                 last_mod = false, location = false, organizer = false, priority = false,
                 seq = false, status = false, summary = false, transp = false, url = false,
                 recurid = false, dtend = false, duration = false;
                 
            while(count > 0) {
                property_temp = (Property*)nextElement(&event_prop_iterator);
               /* class / created / description / geo /
                last-mod / location / organizer / priority /
                seq / status / summary / transp /
                url / recurid /  attach / attendee / categories / comment /
                contact / exdate / rstatus / related /
                resources / rdate /*/
                if(strlen(property_temp->propDescr) == 0) {
              //      printf("here7\n");
                    return INV_EVENT;
                } else if (strlen(property_temp->propName) == 0 || strlen(property_temp->propName) > 200) {
                //    printf("here8\n");
                    return INV_EVENT;
                }
                if(strcmp(property_temp->propName, "CLASS") == 0) {
                    if(class) {
                  //      printf("here9\n");
                        return INV_EVENT;
                    } else {
                        class = true;
                    }
                } else if(strcmp(property_temp->propName, "CREATED") == 0) {
                    
                    if(created) {
                    //    printf("here10\n");
                        return INV_EVENT;
                    } else {
                        created = true;
                    }
                }else if(strcmp(property_temp->propName, "DESCRIPTION") == 0) {
                    if(description) {
                      //  printf("here11\n");
                        return INV_EVENT;
                    } else {
                        description = true;
                    }
                }else if(strcmp(property_temp->propName, "GEO") == 0) {
                    if(geo) {
                      //  printf("here12\n");
                        return INV_EVENT;
                    } else {
                        geo = true;
                    }
                }else if(strcmp(property_temp->propName, "LAST-MOD") == 0) {
                    if(last_mod) {
                    //    printf("here13\n");
                        return INV_EVENT;
                    } else {
                        last_mod = true;
                    }
                }else if(strcmp(property_temp->propName, "LOCATION") == 0) {
                    if(location) {
                     //   printf("here14\n");
                        return INV_EVENT;
                    } else {
                        location = true;
                    }
                }else if(strcmp(property_temp->propName, "ORGANIZER") == 0) {
                    if(organizer) {
                    //    printf("here15\n");
                        return INV_EVENT;
                    } else {
                        organizer = true;
                    }
                }else if(strcmp(property_temp->propName, "PRIORITY") == 0) {
                    if(priority) {
                    //    printf("here16\n");
                        return INV_EVENT;
                    } else {
                        priority = true;
                    }
                }else if(strcmp(property_temp->propName, "SEQ") == 0) {
                    if(seq) {
                    //    printf("here17\n");
                        return INV_EVENT;
                    } else {
                        seq = true;
                    }
                }else if(strcmp(property_temp->propName, "STATUS") == 0) {
                    if(status) {
                    //    printf("here18\n");
                        return INV_EVENT;
                    } else {
                        status = true;
                    }
                }else if(strcmp(property_temp->propName, "SUMMARY") == 0) {
                    if(summary) {
                    //    printf("here19\n");
                        return INV_EVENT;
                    } else {
                        summary = true;
                    }
                }else if(strcmp(property_temp->propName, "TRANSP") == 0) {
                    if(transp) {
                    //    printf("here20\n");
                        return INV_EVENT;
                    } else {
                        transp = true;
                    }
                }else if(strcmp(property_temp->propName, "URL") == 0) {
                    if(url) {
                    //    printf("here21\n");
                        return INV_EVENT;
                    } else {
                        url = true;
                    }
                }else if(strcmp(property_temp->propName, "RECURID") == 0) {
                    if(recurid) {
                    //    printf("here22\n");
                        return INV_EVENT;
                    } else {
                        recurid = true;
                    }
                }else if(strcmp(property_temp->propName, "DTEND") == 0) {
                    dtend = true;
                }else if(strcmp(property_temp->propName, "DURATION") == 0) {
                    duration = true;
                }else if(!check_the_rest(property_temp)){
                   // printf("here23 %s\n", property_temp->propName);
                    return INV_EVENT;
                } else if (duration && dtend){
                   // printf("here24\n");
                    return INV_EVENT;
                }
                count--;
            }
        }
        // end of checking event property 

        count--;
    }
    return OK;
}

bool check_alarm_prop(Property *temp) {
    if(strcmp(temp->propName, "ACTION") == 0 || strcmp(temp->propName, "TRIGGER") == 0) {
        return true;
    } else if (strcmp(temp->propName, "DURATION") == 0 || strcmp(temp->propName, "REPEAT") == 0) {
        return true;
    } else if (strcmp(temp->propName, "ATTACH") == 0) {
        return true;
    }
    return false;
}

bool check_the_rest(Property *temp){
    if(strcmp(temp->propName, "ATTACH") == 0 || strcmp(temp->propName, "ATTENDEE") == 0) {
        return true;
    } else if (strcmp(temp->propName, "CATEGORIES") == 0 || strcmp(temp->propName, "COMMENT") == 0) {
        return true;
    } else if (strcmp(temp->propName, "CONTACT") == 0 || strcmp(temp->propName, "EXDATE") == 0) {
        return true;
    } else if (strcmp(temp->propName, "RSTATUS") == 0 || strcmp(temp->propName, "RELATED") == 0) {
        return true;
    } else if (strcmp(temp->propName, "RESOURCES") == 0 || strcmp(temp->propName, "RDATE") == 0) {
        return true;
    } else if (strcmp(temp->propName, "LAST-MODIFIED") == 0) {
        return true;
        
    } else {
        return false;
    }
}

void deleteCalendar(Calendar* obj) {
    freeList(obj->events);
    freeList(obj->properties);
    free(obj);
}

char* printCalendar(const Calendar* obj) {
    int total_size = 0, propertise_length = 0, size_properties = 0, events_length = 0, size_event = 0;
    char* string = NULL,  *property_temp = NULL, property_string[90000] = "", 
    *event_temp = NULL, event_string[90000] = "";
    ListIterator property, event;

    if(obj == NULL) {
        return NULL;
    }
    property = createIterator(obj->properties);
    propertise_length = getLength(obj->properties);
   

    event =  createIterator(obj->events); 
    events_length = getLength(obj->events);
   
    for(int count = 0; count < propertise_length; count++){
        property_temp = printProperty(nextElement(&property));
        if(property_string == NULL) {
            strcpy(property_string, property_temp);
        } else {
            strcat(property_string, property_temp);
        }
        free(property_temp);
    }
   
    for(int count1= 0; count1 < events_length;count1++){
      
        event_temp = printEvent(nextElement(&event));

        if(event_string == NULL) {
            strcpy(event_string, event_temp);
        } else {
            strcat(event_string, event_temp);
        }
        free(event_temp);
    }

    size_properties = strlen(property_string) + 1;
    size_event = strlen(event_string) + 1;
    total_size = strlen(obj->prodID) + size_properties + size_event + 55;
    string = (char *)malloc(sizeof(char) * total_size);
    sprintf(string, "BEGIN:VCALENDAR\r\nVersion:%.1f\nPRODID:%s\n", obj->version, obj->prodID);
    strcat(string, property_string);
    strcat(string, event_string);
    strcat(string,"END:VCALENDAR\r\n");
    //printf("allocated: %d This is size calendar: %s %ld\n", total_size,string,strlen(string));
    return string;
}
void deleteProperty(void* toBeDeleted) { //not funished yet
    Property* tempProperty;

    if(toBeDeleted == NULL) {
        return;
    }
    tempProperty = (Property *)toBeDeleted;
    free(tempProperty);
}

int compareProperties(const void* first, const void* second){
    Property* tempProperty1;
    Property* tempProperty2;

    if (first == NULL || second == NULL){
		return 0;
	}
	
	tempProperty1 = (Property*)first;
	tempProperty2 = (Property*)second;
	
	return strcmp(tempProperty1->propName, tempProperty2->propName);
}

char* printProperty(void* toBePrinted){
    char* tmpStr = NULL;
	Property* tempProperty;
    int len;

    tempProperty = (Property*)toBePrinted;

    len = strlen(tempProperty->propName) + strlen(tempProperty->propDescr) + 4;
    tmpStr = (char*)malloc(sizeof(char)*len);
    sprintf(tmpStr, "%s:%s\r\n", tempProperty->propName, tempProperty->propDescr);
    //printf("allocated: %d This is size property: %s %ld\n", len,tmpStr,strlen(tmpStr));
	return tmpStr;
}

void deleteEvent(void* toBeDeleted) { //not funished yet
    Event* tempEvent;

    if(toBeDeleted == NULL) {
        return;
    }
    tempEvent = (Event *)toBeDeleted;
    freeList(tempEvent->alarms);
    freeList(tempEvent->properties);
    free(tempEvent);
}

int compareEvents(const void* first, const void* second){
    Event* tempEvent1;
    Event* tempEvent2;

    if (first == NULL || second == NULL){
		return 0;
	}
	
	tempEvent1 = (Event*)first;
	tempEvent2 = (Event*)second;
	
	return strcmp(tempEvent1->UID, tempEvent2->UID);
}

char* printEvent(void* toBePrinted){
    char* tmpStr = NULL, *properties_temp = NULL, property_string[100000] = "",
    *alarm_temp = NULL, alarm_string[10000] = "", DateTime_string[100] = "", *DateTime_temp;
	Event* tempEvent = NULL;
    int len = 0, property_length = 0, size_properties = 0, alarm_length = 0, size_alarm = 0, size_times = 0;
    ListIterator pro_iter, alarm_iter;
   
    tempEvent = (Event*)toBePrinted;

    pro_iter = createIterator(tempEvent->properties);
    property_length = getLength(tempEvent->properties);
   // printf("Event Properties:%d\n",property_length);

    alarm_iter = createIterator(tempEvent->alarms);
    alarm_length = getLength(tempEvent->alarms);


    DateTime_temp = printDate(toBePrinted);
    strcpy(DateTime_string, DateTime_temp);
    free(DateTime_temp);
    
    for(int count = 0; count < property_length; count++) { // get the property
        properties_temp = printProperty(nextElement(&pro_iter));
        if(property_string == NULL) {
            strcpy(property_string, properties_temp);
        } else {
            strcat(property_string, properties_temp);
        }
        free(properties_temp);
    }

    for(int count1 = 0; count1 < alarm_length; count1++) {  // get the alarm
        alarm_temp = printAlarm(nextElement(&alarm_iter));
        if(alarm_string == NULL) {
            strcpy(alarm_string, alarm_temp);
        } else {
            strcat(alarm_string, alarm_temp);
        }
        free(alarm_temp);
    }

    size_properties = strlen(property_string);
    size_alarm = strlen(alarm_string);
    size_times = strlen(DateTime_string);
  
    len = strlen(tempEvent->UID) + size_alarm + size_properties + size_times + 40;

    tmpStr = (char*)malloc(sizeof(char)*len);
	sprintf(tmpStr, "BEGIN:VEVENT\r\nUID:%s\r\n", tempEvent->UID);
    strcat(tmpStr, DateTime_string);
    strcat(tmpStr, property_string);
    strcat(tmpStr, alarm_string);
    strcat(tmpStr, "END:VEVENT\r\n");
    //printf("allocated: %d This is size event: %s %ld\n", len,tmpStr,strlen(tmpStr));
	return tmpStr;
}

char* printAlarm(void* toBePrinted){
    char* tmpStr = NULL, *properties_temp = NULL, property_string[10000] = ""; 
    Alarm *tempAlarm;
    int len, len_property, size_property;
    ListIterator pro_iter;
    
    tempAlarm = (Alarm*)toBePrinted;

    pro_iter = createIterator(tempAlarm->properties);
    len_property = tempAlarm->properties->length;

     for(int count = 0; count < len_property; count++) {
        properties_temp = printProperty(nextElement(&pro_iter));
        if(property_string == NULL) {
            strcpy(property_string, properties_temp);
        } else {
            strcat(property_string, properties_temp);
        }
        free(properties_temp);
    }
    size_property = strlen(property_string) + 1;

    len = strlen(tempAlarm->action)+ strlen(tempAlarm->trigger) + size_property + 50;
    tmpStr = (char*)malloc(sizeof(char)*len);
    sprintf(tmpStr, "BEGIN:VALARM\r\nTRIGGER:%s\r\nACTION:%s\r\n", tempAlarm->trigger, tempAlarm->action);
    strcat(tmpStr, property_string);
    strcat(tmpStr, "END:VALARM\n");
    //printf("allocted: %d This is size Alarm: %s %ld\n", len,tmpStr,strlen(tmpStr));
	return tmpStr;
}

void deleteAlarm(void* toBeDeleted) { //not funished yet
    Alarm* tempAlarm;

    if(toBeDeleted == NULL) {
        return;
    }
    tempAlarm = (Alarm*)toBeDeleted;
    freeList(tempAlarm->properties);
    free(tempAlarm->trigger);
    free(tempAlarm);
}

int compareAlarms(const void* first, const void* second){
    Alarm *first_alarm, *second_alarm;
    
    first_alarm = (Alarm*)first;
    second_alarm = (Alarm*)second;

	return strcmp(first_alarm->action, second_alarm->action);
}

char* printDate(void* toBePrinted){
    char* tmpStr = NULL, *tempBool1 = NULL, *tempBool2 = NULL;
	Event* tempTime;
    int len = 0;

    tempTime = (Event*)toBePrinted;
   
    if(tempTime->startDateTime.UTC == true) {
        tempBool1 = "Z";
    } else {
        tempBool1 = " ";
    }

    if(tempTime->creationDateTime.UTC == true) {
        tempBool2 = "Z";
    } else {
        tempBool2 = " ";
    }
    len = strlen(tempTime->creationDateTime.date) + strlen(tempTime->creationDateTime.time) + strlen(tempTime->startDateTime.date) + 
    strlen(tempTime->startDateTime.time) + strlen(tempBool1) + strlen(tempBool2) + 37;

    tmpStr = (char*)malloc(sizeof(char)*len);

	sprintf(tmpStr, "DTSTART:%sT%s%s\r\nDTSTAMP:%sT%s%s\r\n", tempTime->startDateTime.date, tempTime->startDateTime.time, 
    tempBool1,tempTime->creationDateTime.date, tempTime->creationDateTime.time,tempBool2);
	return tmpStr;
}

void deleteDate(void* toBeDeleted) { //not funished yet
    DateTime* tempDT;

    if(toBeDeleted == NULL) {
        return;
    }
    tempDT = (DateTime*)toBeDeleted;

    free(tempDT);
}

int compareDates(const void* first, const void* second){
    DateTime *first_val, *second_val;

    
    first_val = (DateTime*)first;
    second_val = (DateTime*)second;

    return strcmp(first_val->date, second_val->date);
}

// the flowing are help function
bool isProporty(char *input) {
    if(strstr(input, "BEGIN") != NULL || strstr(input, "PRODID") != NULL || strstr(input, "VERSION") != NULL){
        return false;
    } else if (strstr(input, "DTSTAMP") != NULL || strstr(input, "UID") != NULL || strstr(input, "DTSTART") != NULL){
        return false;
    } else if (strstr(input, "END:VEVENT") != NULL || strstr(input, "END:VCALENDAR") != NULL  ||  strstr(input, "END:VALARM") != NULL) {
        return false;
    } else if(strstr(input, "END:VCALEND") != NULL || strstr(input, "TRIGGER") != NULL || strstr(input, "ACTION") != NULL) {
        return false;
     }else{
        return true;
    }
}

ICalErrorCode check_error(char *temp[], int length) {
    int number_version = 0, number_prodid = 0;
    bool isalarm = false, Ala_trigger = false, Ala_action = false, Ala_End_tag = false;

    bool Cal_Beg_tag = false, Cal_End_tag = false, 
         Cal_Version = false, Cal_Prodid = false, Cal_Event = false;
    bool Eve_UID = false, Eve_DTstart = false,Eve_DTstart_E = false, 
         Eve_DTstamp = false, Eve_DTstamp_E = false ,Eve_End_tag = false;
    bool invalid_version = false, invalid_prodID = false;
    bool IsEvetProperty = false, invalidProperty = false;

   // check if the Calendar is fucked up 
    
    // go through the whole list
    for(int index = 0; index < length; index++) {

        // check if it is invalid Calendar
        if(strstr(temp[index], "BEGIN:VCALENDAR") != NULL) {
            Cal_Beg_tag = true;
        }
        if(strstr(temp[index], "END:VCALENDAR") != NULL) {
            Cal_End_tag = true;
        }

        if( strstr(temp[index], "VERSION") != NULL) {
            char *token = NULL, temp_string[2000] = "";
            float version = 0.0;

            strcpy(temp_string, temp[index]);
            token = strtok(temp_string,":");
            token = strtok(NULL,"\0");
            if(token == NULL) {
                invalid_version = true;
            }
            if(!invalid_version) {
                version = atof(token);
                if(version == 0.0) {
                    invalid_version = true;
                }
            }
            number_version++;
            Cal_Version = true;
        }
        if( strstr(temp[index], "PRODID") != NULL) {
            char *token = NULL, temp_string[2000] = "";

            strcpy(temp_string, temp[index]);
            token = strtok(temp_string,":");
            token = strtok(NULL,"\0");

            if (token == NULL) {
                invalid_prodID = true;
            }

            number_prodid++;
            Cal_Prodid = true;
        }
        if( strstr(temp[index], "BEGIN:VEVENT") != NULL) {
            Cal_Event = true;
            Eve_UID = false, Eve_DTstamp = false, Eve_DTstamp_E = false, Eve_DTstart = false;
            Eve_DTstart_E = false, Eve_End_tag = false;
        }

        //check if it is invalid Event
        if(Cal_Event) {
            if( strstr(temp[index], "UID") != NULL) {
                char *token = NULL, temp_string[2000] = "";

                strcpy(temp_string, temp[index]);
                token = strtok(temp_string,":");
                if(token != NULL) {
                    Eve_UID = true;
                }
            }
            if(strstr(temp[index], "DTSTART") != NULL) {
                Eve_DTstart_E = true;
                char *token = NULL, temp_string[2000] = "";
                int check = 0, total_number = 0;
                bool isT = false;
                strcpy(temp_string, temp[index]);
                if(strstr(temp_string, "Z") != NULL) {
                    check = 1;
                }

                for(int index = 0; index <= strlen(temp_string); index++) {
                    if(isdigit(temp_string[index])) {
                        total_number++;
                    }
                    if(index > 7) {
                        if(temp_string[index] == 'T') {
                            isT = true;
                        }
                    }
                }
                if(total_number != 14 || isT == false) {
                    return INV_DT;
                }

                token = strtok(temp_string,":");
                token = strtok(NULL, "\0");
                if(token != NULL) {
                    token = strtok(token, "T");
                    if(atoi(token) != 0) {
                        if(check == 1) {
                            token = strtok(NULL, "Z");
                            if(atoi(token) != 0) {
                                Eve_DTstart = true;
                            }
                        } else { 
                            if(atoi(token) != 0) {
                                Eve_DTstart = true;
                            }
                        }
                    }
                }
            }

            if(strstr(temp[index], "DTSTAMP") != NULL){
                Eve_DTstamp_E = true;
                char *token = NULL, temp_string[2000] = "";
                int check = 0, total_number = 0;
                bool isT = false;

                strcpy(temp_string, temp[index]);
                if(strstr(temp_string, "Z") != NULL) {
                    check = 1;
                }

                for(int index = 0; index <= strlen(temp_string); index++) {
                    if(isdigit(temp_string[index])) {
                        total_number++;
                    }
                    if(index > 7) {
                        if(temp_string[index] == 'T') {
                            isT = true;
                        }
                    }
                }
                if(total_number != 14) {
                    return INV_DT;
                }
                if(!isT) {
                    return INV_DT;
                }

                token = strtok(temp_string,":");
                token = strtok(NULL, "\0");
                if(token != NULL) {
                    token = strtok(token, "T");
                    if(atoi(token) != 0) {
                        if(check == 1) {
                            token = strtok(NULL, "Z");
                            if(atoi(token) != 0) {
                                Eve_DTstamp = true;
                            }
                        } else { 
                            if(atoi(token) != 0) {
                                Eve_DTstamp = true;
                            }
                        }
                    }
                }
            }
            if(strstr(temp[index], "END:VEVENT") != NULL) {
                Eve_End_tag = true;
            }
            if(strstr(temp[index], "BEGIN:VALARM") != NULL) { // check if there is alarm in the file
                isalarm = true;
                Ala_trigger = false;
                Ala_action = false;
                Ala_End_tag = false;
            }
            if(isalarm) {
                if(strstr(temp[index], "TRIGGER") != NULL) {
                    char *token = NULL, temp_string[2000] = "";

                    strcpy(temp_string, temp[index]);
                    if(strlen(temp[index]) < 9) {
                        return INV_ALARM;
                    }
                    if(strcspn(temp_string, ":") > strcspn(temp_string,";")){
                        token = strtok(temp_string,";");
                    } else {
                        token = strtok(temp_string,":");
                    }
                    token = strtok(NULL,"\0");
                    if(token != NULL) {
                        Ala_trigger = true;
                    }
                }
                if(strstr(temp[index], "ACTION") != NULL) {
                    char *token = NULL, temp_string[2000] = "";

                    strcpy(temp_string, temp[index]);
                    if(strlen(temp[index]) < 8) {
                        return INV_ALARM;
                    }
                    if(strcspn(temp_string, ":") > strcspn(temp_string,";")){
                        token = strtok(temp_string,";");
                    } else {
                        token = strtok(temp_string,":");
                    }
                    token = strtok(NULL,"\0");
                    if(token != NULL) {
                        Ala_action = true;
                    }

                }
                if(strstr(temp[index],"END:VALARM") != NULL) {
                    Ala_End_tag = true;
                }

                if(isProporty(temp[index])) {
                    if(strlen(temp[index]) == 0) {
                        return INV_ALARM;
                    } else {
                        if(strcspn(temp[index], ":") > strcspn(temp[index],";")){
                            if(strcspn(temp[index], ";") + 1 == strlen(temp[index])) {
                                return INV_ALARM;
                            }
                        } else {
                            if(strcspn(temp[index], ":") + 1 == strlen(temp[index])) {
                                return INV_ALARM;
                            }
                        }
                    }
                }

            }
            if(!isalarm) {
                if(isProporty(temp[index])) {
                    IsEvetProperty = true;
                    if(strlen(temp[index]) == 0 || temp[index][0] == ';' || temp[index][0] == ':') {
                        invalidProperty = true;
                    } else {
                        if(strcspn(temp[index], ":") > strcspn(temp[index],";")){
                            if(strcspn(temp[index], ";") + 1 == strlen(temp[index])) {
                                invalidProperty = true;
                            }
                        } else {
                            if(strcspn(temp[index], ":") + 1 == strlen(temp[index])) {
                                invalidProperty = true;
                            }
                        }
                    }
                }
            }

        }
        
    }

    if(!Cal_Beg_tag || !Cal_End_tag || !Cal_Event || !Cal_Version || !Cal_Prodid) {
        return INV_CAL;
    }
    if(invalid_version) {
        return INV_VER;
    }
    if(invalid_prodID) {
        return INV_PRODID;
    }
    if(number_version > 1) {
        return DUP_VER;
    } else if (number_prodid > 1) {
        return DUP_PRODID;
    }

    if(Eve_UID == false || Eve_DTstart_E == false || Eve_DTstamp_E == false || Eve_End_tag == false) {
        return INV_EVENT;
    }
    if(IsEvetProperty) {
        if(invalidProperty) {
            return INV_EVENT;
        }
    }
    if(!Eve_DTstart || !Eve_DTstamp) {
        return INV_DT;
    }
    if(!Eve_DTstart || !Eve_DTstamp) {
        return INV_DT;
    }

    if(isalarm) {
        if(!Ala_action || !Ala_trigger || !Ala_End_tag) {
            return INV_ALARM;
        }
    }

    return OK;
} 

void event_object(Event **ptr, char *info) {
    
    if(strstr(info, "DTSTAMP") != NULL){ // creationDateTime Start
        
        char *token;
        int check = 0;
        if(strstr(info, "Z") != NULL) {
            (*ptr)->creationDateTime.UTC = true;
            check = 1;
        } else {
            (*ptr)->creationDateTime.UTC = false;
        }
        
        token = strtok(info,":");
        token = strtok(NULL, "\0");
    
        
        token = strtok(token, "T");
        strcpy((*ptr)->creationDateTime.date, token);

        if(check == 1) {
            token = strtok(NULL, "Z");
            strcpy((*ptr)->creationDateTime.time, token);
        } else { 
            token = strtok(NULL, "\0");
            strcpy((*ptr)->creationDateTime.time, token);
        }
    }// creationDateTime End
    
    if(strstr(info, "DTSTART") != NULL){ // startDateTime Start
       char *token;
        int check = 0;

        if(strstr(info, "Z") != NULL) {
            (*ptr)->startDateTime.UTC = true;
            check = 1;
        } else {
            (*ptr)->startDateTime.UTC = false;
        }
        
        token = strtok(info,":");
        token = strtok(NULL, "\n");
    
        token = strtok(token, "T");
        strcat(token, "\0");
        strcpy((*ptr)->startDateTime.date, token);

        if(check == 1) {
            token = strtok(NULL, "Z");
            strcpy((*ptr)->startDateTime.time, token);
        } else {
            token = strtok(NULL, "");
            strcat(token, "\0");
            strcpy((*ptr)->startDateTime.time, token);
        }
    }// startDateTime End
    
    if(strstr(info,"UID") != NULL) {
        char *token = NULL;
        token = strtok(info,":");
        token = strtok(NULL, "\0");
        strcpy((*ptr)->UID, token);
    }
} 


void make_property(Property **node_ref, char *string) {
    char *token = NULL, *temp = NULL;
    int size = 0;

    temp = (char*)malloc(sizeof(char)*(strlen(string) + 10 ));
    strcpy(temp, string);
    
    
    if (strcspn(temp, ":") > strcspn(temp,";")) {
        token = strtok(temp, ";");
        token = strtok(NULL, "\0");
        size = strlen(token) + 30;
    } else {
        token = strtok(temp, ":");
        token = strtok(NULL, "\0");
        size = strlen(token) + 30;
    }
    (*node_ref) = (Property*) malloc((sizeof(Property) + sizeof(char)*size));
   
    if (strcspn(string, ":") > strcspn(string,";"))  {
        token = strtok(string, ";");
        strcpy((*node_ref)->propName, token);
        token = strtok(NULL, "\0");
        strcpy((*node_ref)->propDescr, token);
    } else {
        token = strtok(string, ":");
        strcpy((*node_ref)->propName, token);
        token = strtok(NULL, "\0");
        strcpy((*node_ref)->propDescr, token);
    }
    free(temp);
}

// the follwing are for mod 3 functions

char *dtToJSON(DateTime prop) {
    char *toReturn = NULL;

    toReturn = (char*) malloc (sizeof(char) * (42 + strlen(prop.date) + strlen(prop.time)));
    if(!prop.UTC) {
        sprintf(toReturn, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":false}",prop.date, prop.time);
    } else {
        sprintf(toReturn, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":true}",prop.date, prop.time);
    }

    return toReturn;
}

char *eventToJSON(const Event *prop) {
    char *toReturn = NULL, *start = NULL, *summary = NULL, *location = NULL, *organizer = NULL;
    int ala_len = 0, prop_len = 0, temp = 0, len_summary = 0, len_location = 0, len_organizer = 0;
    ListIterator propertyIterator;
    Property *property;

    if (prop == NULL) {
        toReturn = (char*) malloc (sizeof(char) * 3);
        strcpy(toReturn, "{}");
        return toReturn;
    }

    start = dtToJSON(prop->startDateTime);
    ala_len = getLength(prop->alarms);
    prop_len = getLength(prop->properties) + 3;

    if(prop_len > 3) {
        temp = getLength(prop->properties);
        propertyIterator = createIterator(prop->properties);
        while(temp > 0) {
            property =(Property*)nextElement(&propertyIterator);
            if(strcmp(property->propName, "SUMMARY") == 0) {
                summary = (char*)malloc(sizeof(char) *(strlen(property->propDescr) + 5));
                strcpy(summary, property->propDescr);
                len_summary = strlen(summary) + 1;
            }

            if(strcmp(property->propName, "LOCATION") == 0) {
                location = (char*)malloc(sizeof(char) *(strlen(property->propDescr) + 5));
                strcpy(location, property->propDescr);
                len_location = strlen(location) + 1;
            }

            if(strcmp(property->propName, "ORGANIZER") == 0) {
                organizer = (char*)malloc(sizeof(char) *(strlen(property->propDescr) + 5));
                strcpy(organizer, property->propDescr);
                len_organizer = strlen(organizer) + 1;
            }
            temp--;
        }
    }
    
    
    if(summary != NULL) {
        if (location == NULL) {
            if (organizer == NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + len_summary + 80) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"\",\"organizer\":\"\"}",start,prop_len,ala_len,summary);
                free(start);
                free(summary);
            } else if (organizer != NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + len_summary + len_organizer + 80) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"\",\"organizer\":\"%s\"}",start,prop_len,ala_len,summary,organizer);
                free(start);
                free(summary);
                free(organizer);
            }
        } else if (location != NULL) {
            if (organizer == NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + len_summary + len_location + 80) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"%s\",\"organizer\":\"\"}",start,prop_len,ala_len,summary, location);
                free(start);
                free(summary);
                free(location);
            } else if (organizer != NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + len_summary + len_location + len_organizer + 80) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"%s\",\"organizer\":\"%s\"}",start,prop_len,ala_len,summary, location, organizer);
                free(start);
                free(summary);
                free(location);
                free(organizer);
            }
        }
    } else {
        if (location == NULL) {
            if (organizer == NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + 80) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\",\"location\":\"\",\"organizer\":\"\"}",start,prop_len,ala_len);
                free(start);
            } else if (organizer != NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + 80) + len_organizer +sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\",\"location\":\"\",\"organizer\":\"%s\"}",start,prop_len,ala_len,organizer);
                free(start);
                free(organizer);
            }
        } else if (location != NULL) {
            if (organizer == NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + 80 + len_location) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\",\"location\":\"%s\",\"organizer\":\"\"}",start,prop_len,ala_len,location);
                free(start);
                free(location);
            } else if (organizer != NULL) {
                toReturn = (char*)malloc(sizeof(char) * (strlen(start) + 80 + len_location + len_organizer) + sizeof(int) * 2);
                sprintf(toReturn, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\",\"location\":\"%s\",\"organizer\":\"%s\"}",start,prop_len,ala_len,location, organizer);
                free(start);
                free(location);
                free(organizer);
            }
        }
    }
    return toReturn;
}

char *property_json (Event *prop) {

    ListIterator propIterator;
    int length = 0;
    char  organizer[1024] = "None", class[1024] = "None", dtEnd[30] = "None", *toBeReturn;
    propIterator = createIterator(prop->properties);
    length = getLength(prop->properties);
    Property *temp_property;
    
    while (length > 0) {
        temp_property = nextElement(&propIterator);
        if (strcmp(temp_property->propName, "ORGANIZER") == 0) {
            strcpy(organizer,temp_property->propDescr);
        } else if (strcmp(temp_property->propName, "CLASS") == 0) {
            strcpy(class,temp_property->propDescr);
        } else if (strcmp(temp_property->propName, "DTEND") == 0) {
            strcpy(dtEnd, temp_property->propDescr);
        }
        length--;
    }

    toBeReturn = (char*)malloc(sizeof(char)* (strlen(organizer) + strlen(class) + strlen(dtEnd) + 100));
    sprintf(toBeReturn, "{\"organizer\":\"%s\",\"class\":\"%s\",\"dtEnd\":\"%s\"}",organizer,class,dtEnd);
    return toBeReturn;
}

char *alarmToJSON( Alarm *alarm) {
    char *toReturn = NULL, DURATION[1024] = "None", REPEAT[1024] = "None", ATTACH[1024] = "None";
    int length = 0;
    ListIterator property_iterator;
    Property *temp_property;

    property_iterator = createIterator (alarm->properties);
    length = getLength(alarm->properties);

    while (length > 0) {
        temp_property = nextElement(&property_iterator);
        if (strcmp(temp_property->propName, "DURATION") == 0) {
            strcpy(DURATION,temp_property->propDescr);
        } else if (strcmp(temp_property->propName, "REPEAT") == 0) {
            strcpy(REPEAT,temp_property->propDescr);
        } else if (strcmp(temp_property->propName, "ATTACH") == 0) {
            strcpy(ATTACH, temp_property->propDescr);
        }
        length--;
    }

    toReturn = (char*)malloc(sizeof(char) * (strlen(alarm->action) + strlen(alarm->trigger) + strlen(DURATION) 
                                        + strlen(REPEAT) + strlen(ATTACH) + 100));

    sprintf(toReturn, "{\"action\":\"%s\",\"trigger\":\"%s\",\"duration\":\"%s\",\"repeat\":\"%s\",\"attach\":\"%s\"}",
                                alarm->action,alarm->trigger,DURATION, REPEAT, ATTACH);
    return toReturn;

}

char *alarmListToJSON( List *alarm_list) {
    ListIterator iterator;
    List *tempAlarmList;
    Alarm *tempAlarm;
    char *string, *tempString;
    int i;

    tempAlarmList = (List *)alarm_list;
    if(alarm_list == NULL || getLength(tempAlarmList) == 0)
        return "[]";
    
    string = (char*) malloc (sizeof(char ) * 3);
    string = strcpy(string, "[");

    iterator = createIterator(tempAlarmList);
    for(i = 0; i < getLength(tempAlarmList); i++){

        tempAlarm = nextElement(&iterator);
        tempString = alarmToJSON(tempAlarm);

        if(i == getLength(tempAlarmList) - 1){
            string = (char *) realloc(string, strlen(string) + strlen(tempString) + 10);
            strcat(string, tempString);
            strcat(string,"]");
        }
        else{
            string = (char*) realloc(string, strlen(string) + strlen(tempString) + 10);
            strcat(string, tempString);
            strcat(string,",");
        }
    }
    free(tempString);
    return string;
    
}

char *eventListToJSON(const List* eventList) {
    ListIterator iterator;
    List *tempEventList;
    Event *tempEvent;
    char *string, *tempString;
    int i;

    tempEventList = (List *)eventList;
    if(eventList == NULL || getLength(tempEventList) == 0)
        return "[]";
    
    string = (char*) malloc (sizeof(char ) * 3);
    string = strcpy(string, "[");
    iterator = createIterator(tempEventList);
    for(i = 0; i < getLength(tempEventList); i++){
        tempEvent = nextElement(&iterator);
        tempString = eventToJSON(tempEvent);
        if(i == getLength(tempEventList) - 1){
            string = (char *) realloc(string, strlen(string) + strlen(tempString) + 10);
            strcat(string, tempString);
            strcat(string,"]");
        }
        else{
            string = (char*) realloc(string, strlen(string) + strlen(tempString) + 10);
            strcat(string, tempString);
            strcat(string,",");
        }
    }
    free(tempString);
    return string;
}

char *calendarToJSON(const Calendar* cal) {
    char *toReturn = NULL;
    int numProps = 0, numEvents = 0, version = 0;

    if (cal == NULL) {
        toReturn = (char*) malloc (sizeof(char) * 4);
        strcpy(toReturn, "{}");
        return toReturn;
    }
    if(cal->properties != NULL) {
        numProps = getLength(cal->properties) + 2;
    }

    if(cal->events != NULL) {
        numEvents = getLength(cal->events);
    }
    toReturn = (char*)malloc(sizeof(char) * (55 + strlen(cal->prodID)) + sizeof(int) * 3);
    version = (int)cal->version;
    sprintf(toReturn, "{\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}", 
                version, cal->prodID, numProps, numEvents);

    return toReturn;

}

Calendar* JSONtoCalendar(const char* str) {
    int comma_index = 0, start_prodID = 0, index = 0;
    char *temp = NULL;
    Calendar *toReturn = NULL;
    if(str == NULL) {
        return NULL;
    }
    toReturn = (Calendar*) malloc (sizeof(Calendar));

    temp = (char*) malloc (sizeof(char) * strlen(str) + 2);

    strcpy(temp, str);

    comma_index = strcspn(temp, ",");
    start_prodID = comma_index + 11;

    toReturn->version = atof(&temp[comma_index - 1]);

    while(start_prodID < strlen(temp) - 2 && index < 999) {
        toReturn->prodID[index] = temp[start_prodID];
        index++;
        start_prodID++;
    }
    free(temp);
    toReturn->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    toReturn->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
    return toReturn;
}

Event* JSONtoEvent(const char* str) {
    int comma_index = 0, start_UID = 0, index = 0;
    //char *temp = NULL;
    Event *toReturn = NULL;

    if(str == NULL) {
        return NULL;
    }
    toReturn = (Event*) malloc (sizeof(Event));

    //temp = (char*) malloc (sizeof(char) * strlen(str) + 2);
    comma_index = strcspn(str, ":");
    start_UID = comma_index + 2;
    while(start_UID < strlen(str) - 2) {
        toReturn->UID[index] = str[start_UID];
        index++;
        start_UID++;
    }
    toReturn->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
    toReturn->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    return toReturn;
}

ListIterator tempFunction(const List* list){
    ListIterator iter;

    iter.current = list->head;
    
    return iter;
}

void addEvent(Calendar* cal, Event* toBeAdded) {
    
    if(cal != NULL && toBeAdded != NULL) {

        insertBack(cal->events, (void*)toBeAdded);

    }

}



/************ the following are for JavaScript and C Glue****************/

char *glue_code_getFileInfo(char *string_from_JS) {
    Calendar *obj = NULL;
    char *file_name = NULL, *temp;
    ICalErrorCode code;
    int b = 0;

    file_name = malloc (sizeof(char) * ( strlen(string_from_JS) - 1));
    for (int a = 0; a < strlen(string_from_JS); a++) {
        if (string_from_JS[a] != '\"') {
            file_name[b] = string_from_JS[a];
            b++;
        }
    }
    file_name[b] = '\0';
    
    
    code = createCalendar(file_name, &obj);
    
    if (code != OK) {
        free(file_name);
        
        return printError(code);
    }
    

    code  = validateCalendar(obj);
    if (code != OK) {
        return printError(code);
    }

    temp = calendarToJSON(obj);
    deleteCalendar(obj);
    free(file_name);
    
    return temp;
}

char *glue_code_getEventInfo(char *file_name) {
    Calendar *obj = NULL;
    char *temp;
    createCalendar(file_name, &obj);
    temp = eventListToJSON(obj->events);

    return temp;
}

char *glue_code_getAlarmInfo(char *file_name, int event_number) {
    Calendar *obj = NULL;
    char *toBeReturn;
    ListIterator event_list;
    int event_length = 0, start = 1;
    Event *temp_event;

    createCalendar(file_name, &obj);

    event_list = createIterator(obj->events);
    event_length = getLength(obj->events);

    while (start <= event_length) {

        if(start != event_number) {
            temp_event = nextElement(&event_list);
            start++;
        } else {
            temp_event = nextElement(&event_list);
            toBeReturn = alarmListToJSON(temp_event->alarms);
            //printf("From C: %s\n", toBeReturn);
            return toBeReturn;
        }
    }

    return "{\"error\":-1}";
}

char *glue_code_getOtherInfo(char *file_name, int event_number) {
    Calendar *obj = NULL;
    char *toBeReturn;
    ListIterator event_list;
    int event_length = 0, start = 1;
    Event *temp_event;

    createCalendar(file_name, &obj);

    event_list = createIterator(obj->events);
    event_length = getLength(obj->events);

    while (start <= event_length) {

        if(start != event_number) {
            temp_event = nextElement(&event_list);
            start++;
        } else {
            temp_event = nextElement(&event_list);
            toBeReturn = property_json(temp_event);
            //printf("Form C Code: %s\n", toBeReturn);
            return toBeReturn;
        }
    }

    return "{\"error\":-1}";
}

char *make_calendar_file(char *filename, char *version, char *product_id, char *user_id,char *dt_stamp,char *dt_stamp_time, char *dt_start, char *dt_start_time, char *stamp_UTC, char *start_UTC) {
    char *string;
    float temp_float;
    Calendar *cal_obj;
    Event *event;
    ICalErrorCode code;
    
    string = malloc (snprintf(NULL, 0, "./uploads/%s", filename) + 1);
    sprintf (string, "./uploads/%s", filename);
    //printf("Form C: %s %s %s %s %s %s\n", string, version, product_id, user_id, dt_stamp, dt_start);

    cal_obj = malloc (sizeof(Calendar));
    cal_obj->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    cal_obj->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);

    temp_float = atof(version);
    cal_obj->version = temp_float;
    strcpy(cal_obj->prodID, product_id);

    event = malloc (sizeof(Event));
    event->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    event->properties = initializeList(&printProperty,&deleteProperty, &compareProperties);
    strcpy(event->UID, user_id);

    strcpy(event->creationDateTime.date, dt_stamp);
    strcpy(event->creationDateTime.time, dt_stamp_time);
    if (strcmp(stamp_UTC, "True") == 0) {
        event->creationDateTime.UTC = true;
    } else
    {
        event->creationDateTime.UTC = false;
    }
    
    strcpy(event->startDateTime.date, dt_start);
    strcpy(event->startDateTime.time, dt_start_time);
    if (strcmp(start_UTC, "True") == 0) {
        event->startDateTime.UTC = true;
    } else
    {
        event->startDateTime.UTC = false;
    }

    addEvent(cal_obj, event);
    code = validateCalendar(cal_obj);

    if (code == OK) {
        writeCalendar(string, cal_obj);
    }

    deleteCalendar(cal_obj);
   // printf("Form C: %s", printError(code));
    return printError(code);

}

char *add_event_toFile(char *filename, char *uid, char *stamp_date, char *stamp_time, char *stamp_utc,char *start_date, char *start_time, char *start_utc, char *summary) {
    ICalErrorCode code;
    Calendar *cal_obj;
    Event *event;
    Property *property;
    char *string;

    createCalendar(filename, &cal_obj);
    event = malloc (sizeof(Event));
    property = (Property*)malloc (sizeof(Property) + (sizeof(char)* (strlen(summary) + 60)));

    strcpy(property->propName,"SUMMARY");
    strcpy(property->propDescr,summary);

    event->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
    event->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
    //insertBack(event->properties, (void*)property);

    strcpy(event->UID, uid);
    strcpy(event->creationDateTime.date, stamp_date);
    strcpy(event->creationDateTime.time, stamp_time);
    if (strcmp(stamp_utc, "True") == 0) {
        event->creationDateTime.UTC = true;
    } else
    {
        event->creationDateTime.UTC = false;
    }
    
    strcpy(event->startDateTime.date, start_date);
    strcpy(event->startDateTime.time, start_time);
    if (strcmp(start_utc, "True") == 0) {
        event->startDateTime.UTC = true;
    } else
    {
        event->startDateTime.UTC = false;
    }

    addEvent(cal_obj, event);
    code = validateCalendar(cal_obj);

    string = malloc (snprintf(NULL, 0, "./uploads/%s", filename) + 1);
    sprintf (string, "./uploads/%s", filename);

    if (code == OK) {
        writeCalendar(string, cal_obj);
    }

    deleteCalendar(cal_obj);
    return printError(code);
}


