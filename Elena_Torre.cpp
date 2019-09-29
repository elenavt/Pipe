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

int vpipes[20]; //create 20 pipes, as we will have at most 10 vertices. It will also be a global variable
int sizeofbool = sizeof(bool); //variable to know exactly how much space I need for my buffer


void calculation(operation op)
{
    bool buf[sizeofbool];
    
    if(op.input.hasTwoInputs == false) //if the operation is a NOT
    {
        if(op.input.input1.type == 'o') //if NOT is waiting for input from another operation
        {
            close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation
            read(vpipes[2*op.input.input1.opOutput], buf, sizeofbool); //read from output operation
            close(vpipes[2*op.index]); //close read end of main operation type
            buf[sizeofbool] = !buf[sizeofbool];
            write(vpipes[(2*op.index)+1], buf, sizeofbool); //write result
        }
        else
        {
            buf[sizeofbool] = !op.input.input1.inputValue;
            close(vpipes[2*op.index]); //close read end of main operation type
            write(vpipes[(2*op.index)+1], buf, sizeofbool); //write result
        }
       
    }
    else
    {
        if(op.operationType == "AND")
        {
            if(op.input.input1.type == 'o') //if NOT is waiting for input from another operation
            {
                close(vpipes[(2*op.input.input1.opOutput)+1]); //close write end of the pipe of output operation
                read(vpipes[2*op.input.input1.opOutput], buf, sizeofbool); //read from output operation
                bool inp1 = buf[sizeofbool]; //store first input into variable
                if(op.input.input2.type == 'o')
                {
                    close(vpipes[(2*op.input.input2.opOutput)+1]); //close write end of the pipe of output operation
                    read(vpipes[2*op.input.input2.opOutput], buf, sizeofbool); //read from output operation
                    bool inp2 = buf[sizeofbool]; //store first input into variable
                    buf[sizeofbool] = inp1 && inp2; //get result of operation
                    close(vpipes[2*op.index]); //close read end of main operation type
                    write(vpipes[(2*op.index)+1], buf, sizeofbool); //write result

                }
                else
                {
                    bool inp2 = op.input.input2.inputValue;
                    buf[sizeofbool] = inp1 && inp2; //get result of operation
                    close(vpipes[2*op.index]); //close read end of main operation type
                    write(vpipes[(2*op.index)+1], buf, sizeofbool); //write result

                }
            }
        }
        else if(op.operationType == "OR")
        {
            
        }
        else if(op.operationType == "IMPLY")
        {
            
        }
        else
        {
            std::cout<<"invalid operation for vertix: "<<op.index<<std::endl;
        }
        
    }
    

   
}

int main()
{ 
    std::vector<std::string> fileCont;

    for (std::string line; std::getline(std::cin, line); )
    {
       fileCont.push_back(line); 
    }

    std::vector<bool> input_var = inputVarVal(fileCont);
    std::vector<std::string> vertices = vertexVal(fileCont);
    std::vector<operation> operations;

    for(int i = 0; i < vertices.size(); i++)
    {
        operations.push_back(operationCreator(vertices, i));
    }
    
    operation *ptr = &operations[0]; //pointer to the start of the struct array

    for(int i = 3; i < (fileCont.size()-1); i++) //trim and read instruction lines
    {
        std::vector<std::string> instr = instrTrim(fileCont[i]);
        instrReader(ptr, instr, input_var);
    }
    for(int i = 0; i < vertices.size(); i++)
    {
        pipe(&vpipes[2*i]);
    }

    int pid;
    std::vector<bool> vertexResults;
    for(int i = 0; i < vertices.size(); i++)
    {
        pid = fork();
        if(pid == -1)
        {
            perror("error creating pipe");
        }
        else if(pid == 0)
        {
             std::cout<< "child created" << std::endl;
            calculation(operations[i]); //child processes will do the calculations
            break;
        }
    }
    if(pid != 0) //parent process
    {
        std::cout<< "dad created" << std::endl;
        std::cout<< "wait for keystroke ---";
        int x; std::cin >> x; // wait for keystroke
        for(int i = 0; i < vertices.size(); i++)
        {
            close(vpipes[(2*i)+1]); //close the write end of all pipes

        }
        for(int i = 0; i < vertices.size(); i++)
        {
            bool buf[sizeofbool];
            read(vpipes[2*i], buf, sizeofbool); //read from read end of all pipes to get results
            vertexResults.push_back(buf[sizeofbool]); //save results on vertexResults bool vertex
            close(vpipes[2*i]);// close read end
        }
        char inputVarNames[] = {'a','b','c','d','e','f','g','h','i','j'};
        for(int i = 0; i < input_var.size(); i++) // print the values of all input variables
        {
            //std::cout<< inputVarNames[i] << " = " << std::boolalpha << input_var[i] << std::endl;
        }
        for(int i = 0; i < vertexResults.size(); i++)
        {
            //std::cout << "v" << i << " = " << std::boolalpha << vertexResults[i] << std::endl;
        }
    }

    
    return 0;
}


