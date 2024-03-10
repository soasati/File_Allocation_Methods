/*CS 588: Computer System Lab
  Group No.- 14
  Assignment - 6 
  Question - 1
*/

#include<iostream>
#include<bits/stdc++.h>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <time.h>

using namespace std;
//declaring macros
#define TOTAL_BLOCKS 10000 
#define BLOCK_SIZE 4096
#define INDEX_BLOCK_COUNT 10

int disk_size = 10000 * 4096;
int internal_fragmentation =0;
int external_fragmentation =0;


//structure for single block
struct block_structure{
    bool blocks[INDEX_BLOCK_COUNT] = {false};
    struct block_structure *next_block = NULL;
};

typedef struct block_structure block;

//structure for file
struct file_details{
    int file_size;
    block *block_link = NULL;
    int total_blocks;
    int fragment;
};

typedef struct file_details File;

//Disk structure
struct disk_structure{
    unordered_map <int,File*> filemap;
    unordered_map <string,int> name; 
};

typedef struct disk_structure Disk;


//function to generate the report
void report(double access_time){
    ofstream outfile;
    outfile.open("report.txt", std::ios_base::app); // append instead of overwrite
    outfile << "\nIndexed Allocation :";
    outfile << "\nInternal Fragmentation: "<< internal_fragmentation;
    outfile << "\nExternal Fragmentation: "<< external_fragmentation;
    outfile << "\nFile access time: "<<access_time <<"\n";
    outfile.close();
    return;
}

//check if free blocks are available
int check_file_size(Disk *d,int file_size){
    if(file_size > disk_size)
        return -1;
    return 1;
}

//function for creating the file
void file_create(Disk *d,string file_name,int file_id,int file_size,string file_content){
    //calculate internal fragmentation and required blocks
    int requiredBlocks = 0,frag=0;
    if(file_size % 10 == 0){
        requiredBlocks = file_size/10;
    }
    else{
        requiredBlocks = (file_size/10) + 1;
        frag = (requiredBlocks*11)%10;
    }
    //create file 
    ofstream myFile(file_name);
    myFile << file_content;
    myFile.close();
    //create a file structure
    File *file_pointer = new File();
    file_pointer->file_size = file_size;
    file_pointer->total_blocks = requiredBlocks*11;
    file_pointer->fragment = frag;
    //internal fragmentation
    internal_fragmentation += frag;
    //create a index block and assign to file
    block *start = new block();
    file_pointer->block_link = start;
    block *ptr = start;
    //assign dumy index blocks to linked list of blocks
    while(requiredBlocks-- != 1){ //as we have already created first node
        block *node = new block();
        ptr ->next_block = node;
    }
    //assign data blocks into index block
    ptr = start;
    while(ptr){
        for(int i=0;i< INDEX_BLOCK_COUNT;i++)
            ptr->blocks[i] = true;
        ptr = ptr ->next_block;
    }
    //entry into map
    d->filemap[file_id] = file_pointer;
    d->name[file_name] = file_id;

    //decrement the blocks from total blocks
    disk_size -= requiredBlocks*11;
    return;
}

//function for reading the file
void file_read(Disk *d,int file_id,string file_name){
    //traversing the index blocks and accessing the data
    block *startptr = d->filemap[file_id]->block_link;
    while(startptr){
        int i=0;
        while(startptr->blocks[i] == true){
            i++;
        }
        startptr = startptr->next_block;
    }
    //reading from file
    ifstream myFile(file_name);
    string content;
    while(getline(myFile,content))
        cout<<content;
    myFile.close();
    return;
}

//function to delete the file
void file_delete(Disk *d,string file_name){
    //delete the file
    const int delete_file = filesystem::remove(file_name);
    if(delete_file == 1){
        cout << "File successfully deleted\n";
    }
    else{
        cout<< "File Deletion error!\n";
        perror("error: ");
        cout<<endl;
    }
    //add blocks to total blocks
    disk_size += d->filemap[d->name[file_name]]->total_blocks;
    internal_fragmentation -= d->filemap[d->name[file_name]]->fragment;
    //free the blocks from linked list
    block *ptr = d->filemap[d->name[file_name]]->block_link;
    while(ptr){
        block *prev = ptr;
        ptr = ptr->next_block;
        free(prev);
    }
    //delete from map
    d->filemap.erase(d->name[file_name]);
    d->name.erase(file_name);
    
    return;
}

//function to update the file
void file_update(Disk *d,string file_name,int file_id){
    //take the update content as input
    string new_content;
    cout<<"Enter the updated content: \n";
    getchar();
    getline(cin,new_content);
    //check the new and old size of the file
    int new_size = new_content.size();
    int old_size = d->filemap[file_id]->file_size;
    //new size less than/equal to current size
    if(new_size <= old_size){
        file_delete(d,file_name);
        file_create(d,file_name,file_id,new_size,new_content);
    }
    else {
        //new size greater than current size
        int diff = new_size - old_size;
        if(check_file_size(d,diff)){
            file_delete(d,file_name);
            file_create(d,file_name,file_id,new_size,new_content);
        }
        else{
            cout<< "Insufficient space for updation\n";
            external_fragmentation += new_size;
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
        cout<<"Enter the operation to be performed : \n1.Create\n2.Read\n3.Update\n4.Delete\n5.Generate Report\n";
        cin>>choice;
        switch(choice){
            case 1:{    //creating the file
                    cout<<"Enter the Filename ";
                    cin>>file_name;
                    cin.ignore();
                    //take file content as input
                    cout<<"Enter the file content\n";
                    getline(cin,file_content);
                    //cin>>file_content;
                    //calculate the file size
                    int file_size = file_content.size();
                    int reqblocks =0,fragment=0;
                    if(file_size %10 == 0){
                        reqblocks = (file_size /10)*11;
                    }
                    else{
                        reqblocks = (file_size/10) + 1;
                        reqblocks *= 11;
                    }
                    //find the starting index of free space where file can be saved
                    int spaceAvailable = check_file_size(disk_pointer,reqblocks);
                    if(spaceAvailable){
                        id++;
                        //creating the file
                        file_create(disk_pointer,file_name,id,file_size,file_content);
                        cout<<"File created successfully!\n";
                    }
                    else{
                        cout<< "Disk Space not available!\n";
                        external_fragmentation += file_size;
                    }
                break;
            }
            case 2:{//reading from file
                    time_t start;
                    start = clock();
                    cout<<"Enter the Filename ";
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
            case 3:{ //Updating the file
                    cout<<"Enter the name of the file to be updated\n";
                    cin>>file_name;
                    //check if the file exists into the disk
                    if(disk_pointer->name.find(file_name) == disk_pointer->name.end()){
                        cout<<"File Name not found\n";
                    }
                    else{
                        file_update(disk_pointer,file_name,disk_pointer->name[file_name]);
                        cout<<"File Updation Successful" <<endl;
                    }
                break;
            }
            case 4:{ //Deleting the file
                    cout<<"Enter the name of the file to be deleted\n";
                    cin>>file_name;
                    //check if the file exists into the disk
                    if(disk_pointer->name.find(file_name) == disk_pointer->name.end()){
                        cout<<"File Name not found\n";
                    }
                    else{
                        file_delete(disk_pointer,file_name);
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