/*CS 588: Computer System Lab
  Group No.- 14
  Assignment - 6 
  Question - 1
*/

#include<iostream>
#include<bits/stdc++.h>
#include <string>
#include <unistd.h>
#include <time.h>

using namespace std;

//declaring macros
#define TOTAL_BLOCKS 10000
#define BLOCK_SIZE 4096
const int size = TOTAL_BLOCKS * BLOCK_SIZE;

int internal_fragmentation = 0;
int external_fragmentation = 0;


//function to generate the report
void report(double access_time){
    ofstream outfile;
    outfile.open("report.txt", std::ios_base::out); // append instead of overwrite
    outfile << "\nContiguous Allocation :";
    outfile << "\nInternal Fragmentation: "<< internal_fragmentation;
    outfile << "\nExternal Fragmentation: "<< external_fragmentation;
    outfile << "\nFile access time: "<<access_time <<"\n";
    outfile.close();
    return;
}

//structure for disk representation
struct disk_structure{
    int disk_bitmap[(TOTAL_BLOCKS * BLOCK_SIZE)/10] = {0};  //every index represents 10 characters
    unordered_map <int,pair<int,int>> file_position;  //fileid ,start,length
    unordered_map <string,int> name; 
    unordered_map <int,int> frag; //to keep track of fragmentation
    char content[TOTAL_BLOCKS * BLOCK_SIZE];
};

typedef struct disk_structure Disk;

//function for checking the free space
int free_space(Disk *d,int filesize){
    int count =0;
    for(int i= 0 ;i< (TOTAL_BLOCKS * BLOCK_SIZE)/10;i++){
        if(d->disk_bitmap[i] == 0){
            count++;
            if(count >= filesize)
                return i -filesize + 1;
        }
        if(d->disk_bitmap[i] != 0)
            count = 0;
    }
    return -1;
}


//function for creating the file
void file_create(Disk *d,int start_index,int filesize,int fileid,string file_content,string file_name){
    //calculate internal fragmentation
    internal_fragmentation += filesize % 10;
    d->frag[fileid] = filesize % 10;
    //set the bitmap to fileid
    for(int i = start_index,j=0;i<=start_index+filesize;i++,j++){
        d->disk_bitmap[i] = fileid;
        d->content[i] = file_content[j];
    } 
    //save file position
    d->file_position[fileid] = {start_index,filesize};
    d->name[file_name] = fileid;
    //create file
    ofstream myFile(file_name);
    myFile << file_content;
    myFile.close();
    return;
}

//function for reading the file
void file_read(Disk *d,int file_id,string file_name){
    //traverse the file into bitmap
    pair<int,int> file = d->file_position[file_id];
    for(int i = file.first;i<= file.first+file.second;i++){
        int temp = d->disk_bitmap[i];
    }
    cout<<endl;
    //read and print the file
    ifstream myFile(file_name);
    string content;
    while(getline(myFile,content))
        cout<<content;
    myFile.close();
    return;
}   

//function to delete the file
void file_delete(Disk *d,int file_id,string file_name){
    //calculate the internal fragmentation
    pair<int,int> file = d->file_position[file_id];
    internal_fragmentation -= file.second % 10;
    //set the bitmap to 0 for free space
    for(int i = file.first;i<= file.first+file.second;i++){
        d->disk_bitmap[i] = 0;
    }
    //delete the entries from disk
    d->name.erase(file_name);
    d->file_position.erase(file_id);
    return;
}

//function to update the file
void file_update(Disk *d,int file_id,string file_name){
    //take the update content as input
    string new_content;
    cout<<"Enter the updated content: \n";
    getchar();
    getline(cin,new_content);
    //check the new and old size of the file
    int new_size = new_content.size();
    int old_size = d->file_position[file_id].second;
    //new size less than current size
    if(new_size < old_size){
        int i,j=0;
        int start =d->file_position[file_id].first;
        for(i = start;i<= start + new_size;i++){
            d->content[i] = new_content[j];
        }
        for(;i<=old_size;i++){
            d->disk_bitmap[i] = 0;
        }
        //delete old file
        file_delete(d,file_id,file_name);
        //write new file
        file_create(d,start,new_size,file_id,new_content,file_name);
    }
    //new size equal to current size
    else if(new_size == old_size){
        int i,j=0;
        int start =d->file_position[file_id].first;
        for(i = start;i<= start + new_size;i++){
            d->content[i] = new_content[j];
        }
        //delete old file
        file_delete(d,file_id,file_name);
        //write new file
        file_create(d,start,new_size,file_id,new_content,file_name);
    }
    //new size greater than current size
    else if(new_size > old_size){
        //check if new size available
        int diff = new_size - old_size;
        int start =d->file_position[file_id].first;
        int count = 0;
        //check the free space
        for(int i = start+old_size+1;i<=start+ new_size;i++){
            if(d->disk_bitmap[i] == 0){
                count++;
            }
        }
        int index = free_space(d,new_size);
            if(index != -1){
                //space available
                //delete old file
                file_delete(d,file_id,file_name);
                //write new file
                file_create(d,index,new_size,file_id,new_content,file_name);
            }
            else{
                cout<<"File space not available!\n";
                cout<< "File update rejected" <<endl;
                external_fragmentation += new_size;
            }
        if(count >= diff){
            //directly write the file
            int j=0;
            for(int i = start;i<= start + new_size;i++){
                d->content[i] = new_content[j];
            }
            //update bitmap
            for(int i = start+old_size+1;i<=start+ new_size;i++){
                d->disk_bitmap[i] = file_id;
            }
        }
        else{
            //check empty space nd then save
            int index = free_space(d,new_size);
            if(index != -1){
                //space available
                //delete old file
                file_delete(d,file_id,file_name);
                //write new file
                file_create(d,index,new_size,file_id,new_content,file_name);
            }
            else{
                //calculate th external fragmentation
                cout<<"File space not available!\n";
                cout<< "File update rejected" <<endl;
                external_fragmentation += new_size;
            }
        }
    }
    return;
}

//driver code
int main(){
    //creating disk pointer
    Disk *disk_pointer = new Disk();
    if(disk_pointer == NULL){
        perror("Memory allocation to Disk failed");
        exit(0);
    }
    //declaring variables
    string file_name,file_content;
    int id=0;

    double file_access_time = 0;
    while(true){
        char cont;
        int choice;
        //taking choice from user for operations
        cout<<"Enter the operation to be performed :\n1.Create\n2.Read\n3.Update\n4.Delete\n5.Generate Report\n";
        cin>>choice;
        switch(choice){
            case 1:{ //create the file
                    cout<<"Enter the Filename ";
                    cin>>file_name;
                    cin.ignore();
                    //take file content as input
                    cout<<"Enter the file content\n";
                    getline(cin,file_content);
                    //cin>>file_content;
                    //calculate the file size
                    int file_size = file_content.size();
                    //find the starting index of free space where file can be saved
                    //First-fit policy is used for file space allocation
                    int start_index =free_space(disk_pointer,file_size);
                    if(start_index != -1){
                        cout<<"File space available\n";
                        id++;
                        //creating the file
                        file_create(disk_pointer,start_index,file_size,id,file_content,file_name);
                        //calculating internal fragmentation
                        cout<<"File saved successfully\n";
                    }
                    else{
                        //space not avaialable
                        cout<<"File Space not available\n";
                        external_fragmentation += file_size;
                    }
                    
                    break;
            }
            case 2:{//reading file
                    //calculating file accesss time
                    clock_t start;
                    start = clock();
                    //take the file name
                    cout<<"Enter the name of the file to be read\n";
                    cin>>file_name;
                    //check if the file exists into the disk
                    if(disk_pointer->name.find(file_name) == disk_pointer->name.end()){
                        cout<<"File Name not found\n";
                    }
                    else{
                        //file exists
                        file_read(disk_pointer,disk_pointer->name[file_name],file_name);
                        cout<<"\nFile read successfully!\n";
                        time_t end = time(NULL);
                        file_access_time += end - start;
                    }
                    

                    break;
            }
            case 3:{//Updating the file
                    cout<<"Enter the name of the file to be updated\n";
                    cin>>file_name;
                    //check if the file exists into the disk
                    if(disk_pointer->name.find(file_name) == disk_pointer->name.end()){
                        cout<<"File Name not found\n";
                    }
                    else{
                        file_update(disk_pointer,disk_pointer->name[file_name],file_name);
                        cout<<"File Updation Successful" <<endl;
                    }
                    break;
            }
            case 4:{ //deleting the file
                    cout<<"Enter the name of the file to be deleted\n";
                    cin>>file_name;
                    //check if the file exists into the disk
                    if(disk_pointer->name.find(file_name) == disk_pointer->name.end()){
                        cout<<"File Name not found\n";
                    }
                    else{
                        file_delete(disk_pointer,disk_pointer->name[file_name],file_name);
                        cout<<"File deleted successfully\n";
                    }
                    break;
            }
            case 5:{
                //generate the report
                //this will write the fragmentation details and file access time to report.txt
                    report(file_access_time);
                break;
            }
            default:{
                cout<< "\nWrong input!\n";
            }
        }
        cout<<"Do you want to continue? (y/n)";
        cin>>cont;
        if(cont == 'n')
            break;
    }
    return 0;
}