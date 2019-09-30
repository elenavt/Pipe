#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <assert.h>

struct inputType //nested struct that holds information for individal inputs
{
    bool inputValue;//true or false
    char type; // to see if the input is an output from another operation
    int opOutput; //if the input is an output from another operation, what is the index of that operation
};
struct inputInfo //nested struct that holds the information for the input
{
    inputType input1;//information for both inputs
    inputType input2;
    bool hasInput1 = false;//to see if an input 1 has been entered
    bool isFull = false;//to see if it already has two inputs
    bool hasTwoInputs = true;//to see if it needs to inputs (NOT only needs one)
};
struct operation //struct that holds the information for every vertex opetation
{
    int index;
    inputInfo input;
    std::string operationType;

};

int vpipes[20]; //create 20 pipes, as we will have at most 10 vertices. These pipes will handle communications between vertices
int sizeofbool = sizeof(bool); //variable to know exactly how much space I need for my buffer
int vresults[20]; //create 20 pipes that will be used to pipe the results from the child processes to the parent process

//functions:
operation operationCreator(std::vector<std::string> vertices, int i);
/*operationCreator creates an operation struct for every vertex and records whether the operation needs 2 inputs*/
std::vector<std::string> instrTrim(std::string line);
/*instrTrim removes unnecessary characters from the lines of the text file that contain instructions.
It then stores the information in a size 2 array, where the first element always contains the input and the second
always contains the operation that is taking in the input */
void inputInsert(operation *operationPointer, std::vector<bool> boolArray,int operationIdx, int valIndex, bool isOutput);
/* inputInsert takes in a pointer to the operation in the struct array that corresponds to the input being inserted, 
the vector that contains the values of all input variables, the operation index, the int value valIndex and a boolean
representing whether the input comes from a variable or the output of another operation. If the input comes from a 
variable, valIndex indicates the index in the vector on input variables where the input is located. If the input is the output
of another operation, valIndex indicates the index of the operation whose output we need. The function stores all
the information into the operation struct*/
void instrReader(operation *ptr, std::vector<std::string> instrArray, std::vector<bool> boolArray);
/* instrReader takes in a pointer to the operation in the struct array that corresponds to the input being inserted, 
the size 2 array that contains the input and corresponding operation, and the array with the values of all input variables.
The function checks whether the input is a variable or the output of another operation. It then calls the inputInsert function
and passes it the information it needs*/
std::vector<bool> inputVarVal(std::vector<std::string> inp);
/*inputVarVal records the number of input variables that will be used and their values. It returns a vector that contains 
the values of all the variables in order*/
std::vector<std::string> vertexVal(std::vector<std::string> inp);
/*vertexVal records how many vertices we have and what operation type each one is. It returns a string vector with 
the operation types*/
void log(int idx, std::string message);
void calculation(operation op);
/*calculation performs the operations and pipes the results as needed. It takes in every operation struct and it is called
by the child process*/

int main() 
{ 
    /*the input text file is passed as the console input
    example:
    g++ Elena_Torre.cpp
    ./a.out < input.txt*/

    std::vector<std::string> fileCont; //string vector to store each line on the text file

    for (std::string line; std::getline(std::cin, line); )
    {
       fileCont.push_back(line); //each line is an element in this vector
    }

    std::vector<bool> input_var = inputVarVal(fileCont); //store the input variables in a vector
    std::vector<std::string> vertices = vertexVal(fileCont); //store vertex operations in a vector
    std::vector<operation> operations; //create a vector of operation structs

    for(int i = 0; i < vertices.size(); i++)
    {
        operations.push_back(operationCreator(vertices, i)); //create a struct for every vertex
    }
    
    operation *ptr = &operations[0]; //pointer to the start of the struct array

    for(int i = 3; i < (fileCont.size()-1); i++) //trim and read instruction lines
    {
        std::vector<std::string> instr = instrTrim(fileCont[i]);
        instrReader(ptr, instr, input_var);
    }
    for(int i = 0; i < vertices.size(); i++) //open vertex pipes
    {
        pipe(&vpipes[2*i]);
    }
    for(int i = 0; i < vertices.size(); i++) //open result pipes
    {
        pipe(&vresults[2*i]);
    }

    log(-1,"MAIN: Vertices: " + std::to_string(vertices.size()));

    int pid; //process ID
    //int childIdx = 0; 
    for(int i = 0; i < vertices.size(); i++)
    {
        pid = fork(); //fork
        if(pid == -1)
        {
            perror("error creating pipe");
        }
        else if(pid == 0)
        {
            calculation(operations[i]); //child processes will do the calculations and then break out of the loop
            //childIdx = i;
            break;
        }
    }
    if(pid != 0) //parent process
    {

        //log(-1, "waiting for 2 seconds....");
        //sleep(2);

        bool results[10]; //buffer to store all operation results
        for(int i = 0; i < vertices.size(); i++)
        {
            bool buf[sizeofbool]; //buffer to read results
            int x = read(vresults[2*i], buf, sizeofbool);
            if (x==0) std::cout<< "error reading from vertex: "<< i<< std::endl;
            results[i] = buf[0];


        }
         for(int i = 0; i < vertices.size(); i++)
        {
            close(vresults[(2*i)+1]); //close the write and read end of all result pipes
            close(vresults[(2*i)]);
        }
        for(int i = 0; i < vertices.size(); i++)
        {
            close(vpipes[(2*i)+1]); //close the write and read end of all vertex pipes
            close(vpipes[(2*i)]);
        }
        char inputVarNames[] = {'a','b','c','d','e','f','g','h','i','j'};
        for(int i = 0; i < input_var.size(); i++) // print the values of all input variables
        {
            std::cout<< inputVarNames[i] << " = " << std::boolalpha << input_var[i] << std::endl;
        }
        for(int i = 0; i < vertices.size(); i++) //print the results of all operations
        {
            std::cout << "v" << i << " = " << std::boolalpha << results[i] << std::endl;
        }
    }
   
    return 0;
}

operation operationCreator(std::vector<std::string> vertices, int i) //function to create operations
{
    operation operation;
    operation.index = i;//assign index
    operation.operationType = vertices[i];//record operation type
    if(operation.operationType == "NOT") //If it is a NOT operation record that it doesn't need two inputs
    {
        operation.input.hasTwoInputs = false;
    }
    return operation;
}
std::vector<std::string> instrTrim(std::string line)
{
    //use the same method as in the vertexVal function to trim and separate string by spaces
    char chars[] = "v->;";
    for(int i =0; i < 4; i++)
    {
        line.erase(std::remove(line.begin(), line.end(), chars[i]), line.end());
    }
    std::istringstream instrLine(line);
    std::vector<std::string> instrArray;

    for (std::string line; instrLine >> line; )
    {
        instrArray.push_back(line);
    }
    return instrArray;
}
void inputInsert(operation *operationPointer, std::vector<bool> boolArray,int operationIdx, int valIndex, bool isOutput)
{
    operation* myPtr = &operationPointer[operationIdx];//pointer to the operation we are inserting input in

    if(isOutput == false)//function to insert inputs to operation structs. Checks to see if there are empty slots on the struct.
    {
         if(myPtr->input.isFull == false)
        {
            if(myPtr->input.hasInput1 == false)
            {

                myPtr->input.input1.type = 'i';
                myPtr->input.input1.inputValue = boolArray[valIndex];
                myPtr->input.hasInput1 = true;
                if(myPtr->input.hasTwoInputs == false)
                {
                    myPtr->input.isFull = true;
                }
            }
            else
            {
                if(myPtr->input.hasTwoInputs == false)
                {
                    myPtr->input.isFull = true;
                }
                else
                {
                    myPtr->input.input2.type = 'i';
                    myPtr->input.input2.inputValue = boolArray[valIndex];
                    myPtr->input.isFull = true;
                }
                
            }
        }
    }
    else //if the input is an output from another function
    {
        if(myPtr->input.isFull == false)
        {
            if(myPtr->input.hasInput1 == false)
            {
                myPtr->input.input1.type = 'o';
                myPtr->input.input1.opOutput = valIndex;
                myPtr->input.hasInput1 = true;
                if(myPtr->input.hasTwoInputs == false)
                {
                    myPtr->input.isFull = true;
                }
            }
            else
            {
                if(myPtr->input.hasTwoInputs == false)
                {
                    myPtr->input.isFull = true;
                }
                else
                {
                    myPtr->input.input2.type = 'o';
                    myPtr->input.input2.opOutput = valIndex;
                    myPtr->input.isFull = true;
                }
                
            }
        }
        
        
    }
    
}
void instrReader(operation *ptr, std::vector<std::string> instrArray, std::vector<bool> boolArray)
{
    assert(instrArray[0].size() == 1);
    char input = (instrArray[0])[0];//first element in the instruction array is always an input. I save it as a char
    std::stringstream operationInstr(instrArray[1]);//second element is the operation for the input
    int operationIdx; //I save the value of the operation as an int
    operationInstr >> operationIdx;
    std:: string inputOptions = "abcdefghij";//names of input_vars in alphabetical order. I can use the index to get the value from boolArray
    bool isOutput;
    int j = 0;
    for(std::string::size_type i = 0; i< inputOptions.size(); i++)
    {
        if(input == inputOptions[i])//if the input value is alphabetical then insert the corresponding position i
        {
            
            isOutput = false;
            inputInsert(ptr, boolArray, operationIdx, i, isOutput);
            break;
        }
        j++;
    }
    if(j >= inputOptions.size())//to check that it went through all input options
    {
        isOutput = true;//if input value is not alphabetical then it is a reference to the output of another operation
        int outputIdx = input - '0';//change it into int by using ASCII table values
        inputInsert(ptr, boolArray, operationIdx, outputIdx, isOutput);//use inputInsert function
    }

}
std::vector<bool> inputVarVal(std::vector<std::string> inp)
{
    
    std::vector<std::string> varValues;
    std::vector<bool> boolValues;
    //array with the values of the input variables in the order they are declared;
    std::string valLine = inp[1];
    //values for the input variables are declared on the second line of the input file
    valLine.pop_back(); //to remove ; at the end of line
    std::istringstream line(valLine);
    //istringstream function from standard library to separate the values of input_var by spaces
    
    for (std::string valLine; line >> valLine; )
    {
        varValues.push_back(valLine);
        //push the values into array
    }
    //second for loop to store the input variable values as booleans
    for (int i =0; i < varValues.size();i++ )
    {
        if (varValues[i] == "T") 
        {
            boolValues.push_back(true);
        }
        else 
        {
            boolValues.push_back(false);
        }
    }
    return boolValues;
    //return array

}
std::vector<std::string> vertexVal(std::vector<std::string> inp)
{

    std::vector<std::string> vtxValues;
    //array with the values of the vertices saved as strings
    std::string vtxLine = inp[2].substr(7, inp[2].size());
    //third line contains the vertices, with the first 6 characters removed
    char chars[] = "v0123456789;,=";

    //for loop iteraters through the array of characters to be removed and deletes them from the string
    for(int i =0; i < 14; i++)
    {
        vtxLine.erase(std::remove(vtxLine.begin(), vtxLine.end(), chars[i]), vtxLine.end());
    }

    //I use the same method as I did above, separating the string by spaces and pushing each value into a string vector
    std::istringstream line(vtxLine);
    
    for (std::string vxtLine; line >> vtxLine; )
    {
        vtxValues.push_back(vtxLine);
    }
    return vtxValues;
    //return array of strings
}


void log(int idx, std::string message) {
    std::cout<< "["<<idx<<"] "<< message << std::endl;
}

void calculation(operation op)
{
    bool buf1[sizeofbool]; 
    bool buf2[sizeofbool];
    bool bufRes[sizeofbool];//buffers for read/write operations
    
    log(op.index, "Starting...");
    log(op.index, "operation type: " + op.operationType);
    log(op.index, "Input 1 type: " + std::string(1, op.input.input1.type));
    log(op.index, "Input 1 Value: " + std::to_string(op.input.input1.inputValue));
    log(op.index, "Input 1 Index: " + std::to_string(op.input.input1.opOutput));
    log(op.index, "Input 2 type: " + std::string(1, op.input.input2.type));
    log(op.index, "Input 2 Value: " + std::to_string(op.input.input2.inputValue));
    log(op.index, "Input 2 Index: " + std::to_string(op.input.input2.opOutput));

    if(op.operationType == "NOT") //if the operation is a NOT
    {
        if(op.input.input1.type == 'o') //if NOT is waiting for input from another operation
        {

            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]); //close read end of the pipe of output operation
            close(vpipes[2*op.index]); //close read end of main operation
            buf1[0] = !(buf1[0]);
            log(op.index, "NOT Result: " + std::to_string(buf1[0]));
            int x = write(vpipes[(2*op.index)+1], buf1, sizeofbool); //write result
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]); //close write end of main operation\
            
            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], buf1, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else
        {
            buf1[0] = !(op.input.input1.inputValue);
            close(vpipes[2*op.index]); //close read end of main operation
            log(op.index, "NOT Result: " + std::to_string(buf1[0]));
            int x = write(vpipes[(2*op.index)+1], buf1, sizeofbool); //write result
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]); //close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], buf1, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
       
    }
    else if(op.operationType == "AND")
    {
        if(op.input.input1.type == 'i' && op.input.input2.type == 'i') //if both inputs are ready
        {
            bool inp1 = op.input.input1.inputValue;
            bool inp2 = op.input.input2.inputValue;
            bufRes[0] = (inp1 && inp2); //store result of operation in buff
            close(vpipes[2*op.index]);//close read end of main operation
            log(op.index, "AND Result: " + std::to_string(bufRes[0]));
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]); //close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else if(op.input.input1.type == 'o' && op.input.input2.type == 'o') //if both waiting for output from another operation
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation 1
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation 1
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]);//close read end of output operation 1
            bool inp1 = buf1[0]; //store first input into variable

            close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation 2
            int z = read(vpipes[2*op.input.input2.opOutput], buf2, sizeofbool); //read from output operation 2
            if(z == 0) std::cout<< "error on read on second input of operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input2.opOutput]);//close read end of output operation 2
            bool inp2 = buf2[0]; //store first input into variable

            bufRes[0] = (inp1 && inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "AND Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);

        }
        else if (op.input.input1.type == 'o' && op.input.input2.type == 'i')
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation 1
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation 1
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]);//close read end of output operation 1
            bool inp1 = buf1[0]; //store first input into variable

            bool inp2 = op.input.input2.inputValue;

            bufRes[0] = (inp1 && inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
             int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
             log(op.index, "AND Result: " + std::to_string(bufRes[0]));
             if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);


        }
        else if(op.input.input1.type == 'i' && op.input.input2.type == 'o')
        {
            bool inp1 = op.input.input1.inputValue;

            close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation 2
            int y = read(vpipes[2*op.input.input2.opOutput], buf2, sizeofbool); //read from output operation 2
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input2.opOutput]);//close read end of output operation 2
            bool inp2 = buf2[0]; //store first input into variable

            bufRes[0] = (inp1 && inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "AND Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else
        {
            std::cout<< "error"<<std::endl;
        }

    }

    else if(op.operationType == "OR")
    {
        if(op.input.input1.type == 'i' && op.input.input2.type == 'i') //if both inputs are ready
        {
            bool inp1 = op.input.input1.inputValue;
            bool inp2 = op.input.input2.inputValue;
            bufRes[0] = (inp1 || inp2); //store result of operation in buff
            //log(op.index, "result: " + std::to_string(bufRes[sizeofbool]));
            close(vpipes[2*op.index]);//close read end of main operation
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "OR Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]); //close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else if(op.input.input1.type == 'o' && op.input.input2.type == 'o') //if both waiting for output from another operation
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation 1
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation 1
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]);//close read end of output operation 1
            bool inp1 = buf1[0]; //store first input into variable

            close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation 2
            int z = read(vpipes[2*op.input.input2.opOutput], buf2, sizeofbool); //read from output operation 2
            if(z == 0) std::cout<< "error on read on second input of operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input2.opOutput]);//close read end of output operation 2
            bool inp2 = buf2[0]; //store first input into variable

            bufRes[0] = (inp1 || inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "OR Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);

        }
        else if (op.input.input1.type == 'o' && op.input.input2.type == 'i')
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation 1
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation 1
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]);//close read end of output operation 1
            bool inp1 = buf1[0]; //store first input into variable

            bool inp2 = op.input.input2.inputValue;

            bufRes[0] = (inp1 || inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "OR Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);

        }
        else if(op.input.input1.type == 'i' && op.input.input2.type == 'o')
        {
            bool inp1 = op.input.input1.inputValue;

            close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation 2
            int y = read(vpipes[2*op.input.input2.opOutput], buf2, sizeofbool); //read from output operation 2
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input2.opOutput]);//close read end of output operation 2
            bool inp2 = buf2[0]; //store first input into variable

            bufRes[0] = (inp1 || inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "OR Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else 
        {
            std::cout<< "error"<<std::endl;
        }

    }

    else if(op.operationType == "IMPLY")
    {
        if(op.input.input1.type == 'i' && op.input.input2.type == 'i') //if both inputs are ready
        {
            bool inp1 = op.input.input1.inputValue;
            bool inp2 = op.input.input2.inputValue;
            bufRes[0] = (!inp1 || inp2); //store result of operation in buff
            close(vpipes[2*op.index]);//close read end of main operation
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "IMPLY Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]); //close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else if(op.input.input1.type == 'o' && op.input.input2.type == 'o') //if both waiting for output from another operation
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation 1
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation 1
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]);//close read end of output operation 1
            bool inp1 = buf1[0]; //store first input into variable

            close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation 2
            int z = read(vpipes[2*op.input.input2.opOutput], buf2, sizeofbool); //read from output operation 2
            if(z == 0) std::cout<< "error on read on  second input of operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input2.opOutput]);//close read end of output operation 2
            bool inp2 = buf2[0]; //store first input into variable

            bufRes[0] = (!inp1 || inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "IMPLY Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);

        }
        else if (op.input.input1.type == 'o' && op.input.input2.type == 'i')
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation 1
            int y = read(vpipes[2*op.input.input1.opOutput], buf1, sizeofbool); //read from output operation 1
            if(y == 0) std::cout<< "error on read on operation "<< op.index<< std::endl;
            close(vpipes[2*op.input.input1.opOutput]);//close read end of output operation 1
            bool inp1 = buf1[0]; //store first input into variable

            bool inp2 = op.input.input2.inputValue;

            bufRes[0] = (!inp1 || inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "IMPLY Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);

        }
        else if(op.input.input1.type == 'i' && op.input.input2.type == 'o')
        {
            bool inp1 = op.input.input1.inputValue;

            close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation 2
            int y = read(vpipes[2*op.input.input2.opOutput], buf2, sizeofbool); //read from output operation 2
            close(vpipes[2*op.input.input2.opOutput]);//close read end of output operation 2
            bool inp2 = buf2[0]; //store first input into variable

            bufRes[0] = (!inp1 || inp2); //get result of operation
            close(vpipes[2*op.index]); //close read end of main operation type
            int x = write(vpipes[(2*op.index)+1], bufRes, sizeofbool); //write result
            log(op.index, "IMPLY Result: " + std::to_string(bufRes[0]));
            if(x == 0) std::cout<< "error on write on operation "<< op.index<< std::endl;
            close(vpipes[(2*op.index)+1]);//close write end of main operation

            //piping result
            close(vresults[2*op.index]); //close read end of the result pipe
            int r = write(vresults[(2*op.index)+1], bufRes, sizeofbool); //write result
            if(r == 0) std::cout<< "error on write (result pipe) on operation "<< op.index<< std::endl;
            close(vresults[(2*op.index)+1]);
        }
        else 
        {
            std::cout<< "error"<<std::endl;
        }
    }

    else
    {
        std::cout<<"invalid operation for vertix: "<<op.index<<std::endl;
    }

}


