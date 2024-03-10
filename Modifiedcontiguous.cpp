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
int internal_fragmentation =0;
int external_fragmentation =0;


//structure for disk representation
struct disk_structure{
    int disk_bitmap[TOTAL_BLOCKS * BLOCK_SIZE][2] = {0};
    unordered_map <int,pair<int,int>> file_position;  //fileid ,start,length
    unordered_map <string,int> name; 
    unordered_map <int,int> frag;
    char content[TOTAL_BLOCKS * BLOCK_SIZE];
};

typedef struct disk_structure Disk;


//function to generate the report
void report(double access_time){
    ofstream outfile;
    outfile.open("report.txt", std::ios_base::app); // append instead of overwrite
    outfile << "\nModified Contiguous Allocation :";
    outfile << "\nInternal Fragmentation: "<< internal_fragmentation;
    outfile << "\nExternal Fragmentation: "<< external_fragmentation;
    outfile << "\nFile access time: "<<access_time <<"\n";
    outfile.close();
    return;
}

//function for checking the free space
int free_space(Disk *d,int filesize){
    int count =0;
    for(int i= 0 ;i< TOTAL_BLOCKS * BLOCK_SIZE;i++){
        if(d->disk_bitmap[i][0] == 0){
            count++;
            if(count >= filesize)
                return 1;
        }
        // if(d->disk_bitmap[i] != 0)
        //     count = 0;
    }
    return -1;
}

//function for checking the free space for linking next free space
vector<int> next_free_space(Disk *d,int filesize,int file_id){
    int count =0;
    vector<int> start;
    for(int i= 0 ;count < filesize;i++){
        if(d->disk_bitmap[i][0] == 0){
            start.emplace_back(i);
            count++;
            d->disk_bitmap[i][0] = file_id;
            d->disk_bitmap[i][1] = -1;
        }
    }
    return start;
}

//function for creating the file
void file_create(Disk *d,int filesize,int fileid,string file_content,string file_name){
    //calculate internal fragmentation
    internal_fragmentation += filesize % 10;
    d->frag[fileid] = filesize % 10;
    //add links to next free space
    vector<int> next_index = next_free_space(d,filesize,fileid);
    int len = next_index.size();
    //add the links to starting index of next free block
    for(int i = 1;i<len;i++){
        if((next_index[i] - next_index[i-1]) != 1)
            d->disk_bitmap[i-1][1] = next_index[i];
    }
    //save file position
    d->file_position[fileid] = {next_index[0],filesize};
    d->name[file_name] = fileid;
    //file creation
    ofstream myFile(file_name);
    myFile << file_content;
    myFile.close();
    return;
}

//function to delete the file
void file_delete(Disk *d,int file_id,string file_name){
    //deleting the file
    const int delete_file = filesystem::remove(file_name);
    if(delete_file == 1){
        cout << "File successfully deleted\n";
    }
    else{
        cout<< "File Deletion error!\n";
        perror("error: ");
        cout<<endl;
    }
    pair<int,int> file = d->file_position[file_id];
    int start_index = file.first,file_len = file.second;
    //calculate the internal fragmentation
    internal_fragmentation -= file.second % 10;
    int i=start_index,count=0;
    //set the bitmap to 0 for free space
    while(count < file_len){
        while(d->disk_bitmap[i][0] == file_id && d->disk_bitmap[i][1] == -1){
            d->disk_bitmap[i][0] = 0;
            d->disk_bitmap[i][1] = 0;
            count++;
            i++;
        }
        i = d->disk_bitmap[i][1];
    }
    return ;
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
    //new size less than/equal to current size
    if(new_size <= old_size){
        file_delete(d,file_id,file_name);
        file_create(d,new_size,file_id,new_content,file_name);
    }
    else {
        //new size greater than current size
        int diff = new_size - old_size;
        if(free_space(d,new_size)){
            file_delete(d,file_id,file_name);
            file_create(d,new_size,file_id,new_content,file_name);
        }
        else{
            cout<< "Insufficient space for updation\n";
            external_fragmentation += new_size;
        }
    }
    return;
}


//function for reading the file
void file_read(Disk *d,int file_id,string file_name){
    pair<int,int> file = d->file_position[file_id];
    int start_index = file.first,file_len = file.second;
    int i=start_index,count=0;
    //traverse the file into bitmap
    while(count < file_len){
        while(d->disk_bitmap[i][0] == file_id && d->disk_bitmap[i][1] == -1){
            count++;
            i++;
        }
        i = d->disk_bitmap[i][1];
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
        cout<<"Enter the operation to be performed : \n1.Create\n2.Read\n3.Update\n4.Delete\n5.Generate Report\n";
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
                    //check if free space is available
                    int start_index =free_space(disk_pointer,file_size);
                    if(start_index != -1){
                        cout<<"File space available\n";
                        id++;
                        //creating the file
                        file_create(disk_pointer,file_size,id,file_content,file_name);
                        cout<<"File saved successfully\n";
                    }
                    else{
                        cout<<"File Space not available\n";
                        //calculate the external fragmetation
                        external_fragmentation += file_size;
                    }
                    
                    break;
            }
            case 2:{//reading file
                    time_t start;
                    start = clock();
                    cout<<"Enter the name of the file to be read\n";
                    cin>>file_name;
                     //check if the file exists into the disk
                    if(disk_pointer->name.find(file_name) == disk_pointer->name.end()){
                        cout<<"File Name not found\n";
                    }
                    else{
                        file_read(disk_pointer,disk_pointer->name[file_name],file_name);
                        cout<<"\nFile read successfully!\n";
                        time_t end = time(NULL);
                        //calculate the file access time of files
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