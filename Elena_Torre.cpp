#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>

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
    bool output;
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
    char input = (instrArray[0])[0];//first element in the instruction array is always an input. I save it as a char
    std::stringstream operationInstr(instrArray[1]);//second element is the operation for the input
    int operationIdx; //I save the value of the operation as an int
    operationInstr >> operationIdx;
    char inputOptions[] = "abcdefghij";//names of input_vars in alphabetical order. I can use the index to get the value from boolArray
    bool isOutput;
    for(int i = 0; i< 10; i++)
    {
        if(input == inputOptions[i])//if the input value is alphabetical then insert the corresponding position i
        {
            isOutput = false;
            inputInsert(ptr, boolArray, operationIdx, i, isOutput);
            break;
        }
        i++;
    }
    isOutput = true;//if input value is not alphabetical then it is a reference to the output of another operation
    int outputIdx = input - '0';//change it into int by using ASCII table values
    inputInsert(ptr, boolArray, operationIdx, outputIdx, isOutput);//use inputInsert function



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
    //int vtxNumb = std::count(inp[2].begin(), inp[2].end(), ',') +1;
    //number of commas +1 is the number of vertices

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

    for(int i = 3; i < (fileCont.size()-1); i++)
    {
        std::vector<std::string> instr = instrTrim(fileCont[i]);
        instrReader(ptr, instr, input_var);
    }
    for(int i = 0; i < operations.size(); i++)
    {
        std::cout<<operations[i].input.input2.inputValue<<std::endl;
    }
    
    return 0;
}


