#include "Backend_x86.h"
class Program Code;

//TODO Добавить работу с числами с плавающей точкой

void Backend_x86(Branch *Root)
{
    Make_ELF();

    Find_Functions(Root);

    Explore_Tree_x86(Root);

    Code.Write_Down();

    //Code.Dump();

    Code.~Program();

}


void Explore_Tree_x86(Branch* Node)
{
    assert(Node);

    if (NTYPE == MODE_SYSTEM_OP){

        System_OP_Switch_x86(Node);

        if (NTYPE != MODE_CONNECTIONS &&
            NTYPE != MODE_NIL &&
            Node->Elem->ElemData != oper)

            return;
    }

    if (NTYPE == MODE_FUNCTION){
        Functions_x86(Node);
        return;
    }

//    if (Node->Elem->Name != nullptr){
//        if (strcmp(Node->Elem->Name, "корешок") == 0){
//            Explore_Tree(Node->Left);
//            ASM_MATH_FUNC(Node);
//            return;
//        }
//    }

    if (Node->Left != nullptr)
        Explore_Tree_x86(Node->Left);

    if (NL == nullptr && NR == nullptr){
        My_Switch_x86(Node);
        return;
    }

    if (Node->Right != nullptr)
        Explore_Tree_x86(Node->Right);

    My_Switch_x86(Node);
}


void My_Switch_x86 (Branch *Node)
{
    assert(Node);

    switch (NTYPE){

        case MODE_MATH_OP:     {Math_OP_x86(Node);                                       break;}

        case MODE_SYSTEM_OP:   {System_OP_Switch_x86(Node);                              break;}

        case MODE_CONNECTIONS: {                                                         break;}

        case MODE_NUMBER:      {Number_x86(Node);                                        break;}

        case MODE_VARIABLE:    {Variables_x86(Node);                                     break;}

        case MODE_MATH_FUNC:   {Math_Func_x86(Node);                                     break;}

        case MODE_NIL:         {                                                         break;}

        case MODE_FUNCTION:    {Functions_x86(Node);                                     break;}


        default: {fprintf(stderr, "Problem occurred in My_Switch_x86, please check input.\n"
                                  "Type - %d\nData - %d", NTYPE, ND); abort();}

    }
}

void System_OP_Switch_x86(Branch *Node)
{
    switch (ND) {

        case if_else: {                                     return;}

        case oper:    {                                     return;}

        case '=':     {Assignment_x86(Node);                return;}

        case ret:     {Variables_x86(NL); Ret_x86(Node);    return;}

        case print:   {Variables_x86(NL); Print_x86();      return;}

        case scan:    {Scan_x86(); Assignment_x86(Node);    return;}

        case If:      {
                       My_Switch_x86(NL->Left);
                       My_Switch_x86(NL->Right);

                       Math_OP_x86(NL);

                       Code.Insert(0); //Space for Mark`s address
                       Code.Insert(0);
                       Code.Insert(0);
                       Code.Insert(0);

                       long int Pos_1 = Code.GetCurPos();

                       Explore_Tree_x86(NR->Left);


                       Code.Insert(233); //jmp

                       Code.Insert(0); //Space for Mark`s address
                       Code.Insert(0);
                       Code.Insert(0);
                       Code.Insert(0);
                       long int Pos_2 = Code.GetCurPos();

                       Code.Edit_Address(Code.GetCurPos() - Pos_1, Pos_1 - 4);

                       Explore_Tree_x86(NR->Right);

                       Code.Edit_Address(Code.GetCurPos() - Pos_2, Pos_2 - 4);

                       return;}



        default: {fprintf(stderr, "Unknown System operator\n%d\n", ND); abort();}
    }
}

void Number_x86(Branch *Node)
{
    Code.Insert(0x68);
    Code.Insert(ND * 100);
    Code.Insert((100 *ND) >> 8);
    Code.Insert((100 *ND) >> 16);
    Code.Insert((100 *ND) >> 24);
}

void Assignment_x86(Branch *Node)
{
    int Shift = Code.Function[Code.Cur_Func].Find_Variable(NLN);
    Explore_Tree_x86(NR);
    if (NR->Elem->Type != MODE_FUNCTION)
        Code.Insert(88); // pop rax

    Code.Insert(0x48); // mov [rbp - Shift], rax
    Code.Insert(0x89);
    Code.Insert(0x85);
    Code.Insert(256 - Shift);
    Code.Insert(0xFF);
    Code.Insert(0xFF);
    Code.Insert(0xFF);
}

void Ret_x86(Branch *Node)
{
    Code.Insert(0x58); //pop rax
}


void Find_Functions(Branch *Node) {
    for (Branch *Another_Node = NR; Another_Node->Left != nullptr; Another_Node = Another_Node->Right) {
        if (Another_Node->Left->Elem->Type == MODE_FUNCTION) {
            Code.Function[Code.Num_Func].Name = Another_Node->Left->Elem->Name;
            int Shift = 8;
            Scan_Function(Another_Node->Left, &Shift);
            Code.Num_Func++;
        }
    }
}

void Scan_Function(Branch *Node, int *Shift)
{
    if (NL != nullptr)
        Scan_Function(NL, Shift);
    if (NR != nullptr)
        Scan_Function(NR, Shift);

    if (NR == nullptr && NL == nullptr){
        if (NT == MODE_VARIABLE &&
        Code.Function[Code.Num_Func].Find_Variable(NN) == NoSuchVars){
            Code.Function[Code.Num_Func].Variables[Code.Function[Code.Num_Func].Amount_Variables].Shift = *Shift;
            Code.Function[Code.Num_Func].Variables[Code.Function[Code.Num_Func].Amount_Variables].Name = NN;
            Code.Function[Code.Num_Func].Amount_Variables++;
            *Shift += 8;
        }
    }
}

void Functions_x86(Branch *Node)
{

    if (Node->Parent->Parent->Parent == nullptr){ // main

        Code.Cur_Func = Code.Which_Func(Node);

        int Shift = Code.Function[Code.Cur_Func].Amount_Variables;

        Code.Insert(0x55); //push rbp

        Code.Insert(0x48); //mov rbp, rsp
        Code.Insert(0x89);
        Code.Insert(0xE5);

        Code.Insert(0x48); // sub rsp, Shift
        Code.Insert(0x83);
        Code.Insert(0xEC);
        Code.Insert(8 * Shift);

        Explore_Tree_x86(NR);

        Code.Insert(0x48); //mov rsp, rbp
        Code.Insert(0x89);
        Code.Insert(0xEC);

        Code.Insert(0x5D); //pop rbp

        Code.Exit();

        return;
    }

    if (Node->Parent->Elem->Type == MODE_CONNECTIONS &&
        Node->Parent->Elem->ElemData == ';') {

        Code.Cur_Func = Code.Which_Func(Node);

        Code.Function[Code.Cur_Func].Address = Code.GetCurPos();

        Code.Insert(0x41); //pop r14
        Code.Insert(0x5E);

        Code.Insert(0x48); //sub rsp, 8 * amount_loc
        Code.Insert(0x83);
        Code.Insert(0xEC);
        Code.Insert(8 * (Code.Function[Code.Cur_Func].Amount_Variables - GetAmountOfExternVars(NL)));

        Code.Insert(0x41); //push r14
        Code.Insert(0x56);

        Code.Insert(0x55); //push rbp

        Code.Insert(0x48); //mov rbp, rsp
        Code.Insert(0x89);
        Code.Insert(0xE5);

        Code.Insert(0x48); //add rbp, 16 + 8 * (amount_loc + amount_extern)
        Code.Insert(0x83);
        Code.Insert(0xC5);
        Code.Insert(16 + 8 * (Code.Function[Code.Cur_Func].Amount_Variables));

        Explore_Tree_x86(NR);

        Code.Insert(0x5D); //pop rbp

        Code.Insert(0x41); //pop r14
        Code.Insert(0x5E);

        Code.Insert(0x48); // add rsp, 8 * (amount_loc + amount_extern)
        Code.Insert(0x83);
        Code.Insert(0xC4);
        Code.Insert(8 * (Code.Function[Code.Cur_Func].Amount_Variables));

        Code.Insert(0x41); // push r14
        Code.Insert(0x56);

        Code.Insert(0xC3); // ret

        return;
    }

    Explore_Tree_x86(NL);

    Code.Insert(0xE8); //call

    Code.Insert(0x00); //call address
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);

    Code.Function[Code.Which_Func(Node)].Calls[Code.Function[Code.Which_Func(Node)].Call_Amount++] = Code.GetCurPos();
}

void Variables_x86(Branch *Node)
{
    if (NPE->Type == MODE_SYSTEM_OP && (NPE->ElemData == scan || NPE->ElemData == '='))
        return;

    int Shift = Code.Function[Code.Cur_Func].Find_Variable(NN);

    if (Shift == NoSuchVars) {
        printf("Variable \"%s\" was not found!\n", NN);
        abort();
    }

    Code.Insert(255); //push [rbp - Shift]
    Code.Insert(117);
    Code.Insert(256 - Shift);
}

int GetAmountOfExternVars(Branch *Node)
{
    int Sum = 0;

    for (Branch* AnotherNode = Node; AnotherNode != nullptr; AnotherNode = AnotherNode->Left)
        if (AnotherNode->Right != nullptr) {
            if (AnotherNode->Right->Elem->Type == MODE_VARIABLE)
                Sum++;
        }
    return Sum;
}

void Math_OP_x86(Branch *Node)
{
    switch (ND){

        case '+': {add();          break;}

        case '-': {sub();         break;}

        case '/': {div();          break;}

        case '*': { mul();         break;}

        case '<': {
                    Compare_x86();
                    Code.Insert(0x48); //cmp rax, 3100h
                    Code.Insert(0x3D);
                    Code.Insert(0x00);
                    Code.Insert(0x31);
                    Code.Insert(0x00);
                    Code.Insert(0x00);

                    Code.Insert(0x0F); //je
                    Code.Insert(0x84);
                    break;}

        case '=': {
                    Compare_x86();
                    Code.Insert(0x48); //cmp rax, 7000h
                    Code.Insert(0x3D);
                    Code.Insert(0x00);
                    Code.Insert(0x70);
                    Code.Insert(0x00);
                    Code.Insert(0x00);

                    Code.Insert(0x0F); //jne
                    Code.Insert(0x85);
                    break;}

        case '>': {
                    Compare_x86();
                    Code.Insert(0x48); //cmp rax, 3100h
                    Code.Insert(0x3D);
                    Code.Insert(0x00);
                    Code.Insert(0x30);
                    Code.Insert(0x00);
                    Code.Insert(0x00);

                    Code.Insert(0x0F); //je
                    Code.Insert(0x84);
                    break;}


        default: {fprintf(stderr, "Unknown Math operator\n[%d] %c\n", ND, ND); abort();}

    }
}

void Compare_x86()
{
    Code.Insert(0x48); //xor rax, rax
    Code.Insert(0x31);
    Code.Insert(0xC0);

    Code.Insert(0x9B); //wait

    Code.Insert(0xDB); //finit
    Code.Insert(0xE3);

    Code.Insert(0xDB); //fild dword [rsp + 8]
    Code.Insert(0x44);
    Code.Insert(0x24);
    Code.Insert(0x08);

    Code.Insert(0xDB); //fild dword [rsp]
    Code.Insert(0x04);
    Code.Insert(0x24);

    Code.Insert(0xD8); //fcom st(1)
    Code.Insert(0xD1);

    Code.Insert(0x9B); //wait

    Code.Insert(0xDF); //fnswtsw ax
    Code.Insert(0xE0);

    Code.Insert(0x48); //add rsp, 16
    Code.Insert(0x83);
    Code.Insert(0xC4);
    Code.Insert(0x10);
}

void Math_Func_x86(Branch *Node)
{

    Code.Insert(0x6A); //push 10
    Code.Insert(0x0A);

    Code.Insert(0x9B); //fwait

    Code.Insert(0xDB); //finit
    Code.Insert(0xE3);

    Code.Insert(0xDB);  //fild DWORD [rsp]
    Code.Insert(0x44);
    Code.Insert(0x24);
    Code.Insert(0x08);

    Code.Insert(0xD9);  //fsqrt
    Code.Insert(0xFA);

    Code.Insert(0x9B);  //fwait

    Code.Insert(0xDA); //fimul dword [rsp]
    Code.Insert(0x0C);
    Code.Insert(0x24);

    Code.Insert(0xD9); //frndint
    Code.Insert(0xFC);

    Code.Insert(0x48); //add rsp, 8
    Code.Insert(0x83);
    Code.Insert(0xC4);
    Code.Insert(0x08);

    Code.Insert(0x48); //mov qword [rsp], 0
    Code.Insert(0xC7);
    Code.Insert(0x04);
    Code.Insert(0x24);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);

    Code.Insert(0xDB); //fist dword [rsp]
    Code.Insert(0x14);
    Code.Insert(0x24);
}

void Scan_x86()
{
    Code.Insert(0x56);
    Code.Insert(0x57);
    Code.Insert(0x41);
    Code.Insert(0x51);
    Code.Insert(0x52);
    Code.Insert(0x51);
    Code.Insert(0x55);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xE5);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xEC);
    Code.Insert(0x10);
    Code.Insert(0xBF);
    Code.Insert(0x02);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xE6);
    Code.Insert(0xBA);
    Code.Insert(0x10);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x48);
    Code.Insert(0x31);
    Code.Insert(0xC0);
    Code.Insert(0x0F);
    Code.Insert(0x05);
    Code.Insert(0x48);
    Code.Insert(0x31);
    Code.Insert(0xC9);
    Code.Insert(0x8A);
    Code.Insert(0x16);
    Code.Insert(0x80);
    Code.Insert(0xFA);
    Code.Insert(0x2D);
    Code.Insert(0x75);
    Code.Insert(0x05);
    Code.Insert(0xB9);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x48);
    Code.Insert(0xFF);
    Code.Insert(0xC6);
    Code.Insert(0x8A);
    Code.Insert(0x16);
    Code.Insert(0x80);
    Code.Insert(0xFA);
    Code.Insert(0x0A);
    Code.Insert(0x75);
    Code.Insert(0xF6);
    Code.Insert(0xBF);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x4D);
    Code.Insert(0x31);
    Code.Insert(0xC9);
    Code.Insert(0x48);
    Code.Insert(0xFF);
    Code.Insert(0xCE);
    Code.Insert(0x48);
    Code.Insert(0x31);
    Code.Insert(0xC0);
    Code.Insert(0x8A);
    Code.Insert(0x06);
    Code.Insert(0x3C);
    Code.Insert(0x2E);
    Code.Insert(0x74);
    Code.Insert(0xF4);
    Code.Insert(0x2C);
    Code.Insert(0x30);
    Code.Insert(0x48);
    Code.Insert(0xF7);
    Code.Insert(0xE7);
    Code.Insert(0x49);
    Code.Insert(0x01);
    Code.Insert(0xC1);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xC8);
    Code.Insert(0x48);
    Code.Insert(0x01);
    Code.Insert(0xE0);
    Code.Insert(0x48);
    Code.Insert(0x39);
    Code.Insert(0xC6);
    Code.Insert(0x74);
    Code.Insert(0x10);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xF8);
    Code.Insert(0xBF);
    Code.Insert(0x0A);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x48);
    Code.Insert(0xF7);
    Code.Insert(0xE7);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xC7);
    Code.Insert(0xEB);
    Code.Insert(0xD1);
    Code.Insert(0x4C);
    Code.Insert(0x89);
    Code.Insert(0xC8);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xF9);
    Code.Insert(0x01);
    Code.Insert(0x75);
    Code.Insert(0x09);
    Code.Insert(0x4D);
    Code.Insert(0x31);
    Code.Insert(0xC9);
    Code.Insert(0x49);
    Code.Insert(0x29);
    Code.Insert(0xC1);
    Code.Insert(0x4C);
    Code.Insert(0x89);
    Code.Insert(0xC8);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xC4);
    Code.Insert(0x10);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xEC);
    Code.Insert(0x5D);
    Code.Insert(0x59);
    Code.Insert(0x5A);
    Code.Insert(0x41);
    Code.Insert(0x59);
    Code.Insert(0x5F);
    Code.Insert(0x5E);
    Code.Insert(0x50);
}

void Print_x86()
{
    Code.Insert(0x58);
    Code.Insert(0x55);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xe5);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xec);
    Code.Insert(0x08);
    Code.Insert(0x4d);
    Code.Insert(0x31);
    Code.Insert(0xff);
    Code.Insert(0xbb);
    Code.Insert(0x0a);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xf8);
    Code.Insert(0x00);
    Code.Insert(0x74);
    Code.Insert(0x25);
    Code.Insert(0x48);
    Code.Insert(0x3d);
    Code.Insert(0xfe);
    Code.Insert(0xff);
    Code.Insert(0xff);
    Code.Insert(0x7f);
    Code.Insert(0x77);
    Code.Insert(0x22);
    Code.Insert(0x48);
    Code.Insert(0x31);
    Code.Insert(0xd2);
    Code.Insert(0x48);
    Code.Insert(0xf7);
    Code.Insert(0xf3);
    Code.Insert(0x48);
    Code.Insert(0x39);
    Code.Insert(0xd0);
    Code.Insert(0x74);
    Code.Insert(0x65);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xc2);
    Code.Insert(0x30);
    Code.Insert(0x49);
    Code.Insert(0x89);
    Code.Insert(0xe1);
    Code.Insert(0x4d);
    Code.Insert(0x01);
    Code.Insert(0xf9);
    Code.Insert(0x41);
    Code.Insert(0x88);
    Code.Insert(0x11);
    Code.Insert(0x49);
    Code.Insert(0xff);
    Code.Insert(0xc7);
    Code.Insert(0xeb);
    Code.Insert(0xe3);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xc2);
    Code.Insert(0xeb);
    Code.Insert(0xe9);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xc2);
    Code.Insert(0x48);
    Code.Insert(0x31);
    Code.Insert(0xc0);
    Code.Insert(0x48);
    Code.Insert(0x29);
    Code.Insert(0xd0);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xe6);
    Code.Insert(0xc7);
    Code.Insert(0x04);
    Code.Insert(0x24);
    Code.Insert(0x2d);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x50);
    Code.Insert(0xb8);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xbf);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xba);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x0f);
    Code.Insert(0x05);
    Code.Insert(0x58);
    Code.Insert(0x4d);
    Code.Insert(0x31);
    Code.Insert(0xff);
    Code.Insert(0xeb);
    Code.Insert(0xb3);
    Code.Insert(0xb8);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xba);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xbf);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x49);
    Code.Insert(0x89);
    Code.Insert(0xe2);
    Code.Insert(0x4d);
    Code.Insert(0x01);
    Code.Insert(0xea);
    Code.Insert(0x49);
    Code.Insert(0xff);
    Code.Insert(0xc2);
    Code.Insert(0x41);
    Code.Insert(0xc6);
    Code.Insert(0x02);
    Code.Insert(0x2e);
    Code.Insert(0x4c);
    Code.Insert(0x89);
    Code.Insert(0xd6);
    Code.Insert(0x0f);
    Code.Insert(0x05);
    Code.Insert(0xeb);
    Code.Insert(0x0f);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xf8);
    Code.Insert(0x00);
    Code.Insert(0x75);
    Code.Insert(0x95);
    Code.Insert(0x4d);
    Code.Insert(0x89);
    Code.Insert(0xfd);
    Code.Insert(0x49);
    Code.Insert(0x83);
    Code.Insert(0xff);
    Code.Insert(0x02);
    Code.Insert(0x74);
    Code.Insert(0xce);
    Code.Insert(0xb8);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xe6);
    Code.Insert(0x4c);
    Code.Insert(0x01);
    Code.Insert(0xfe);
    Code.Insert(0x48);
    Code.Insert(0xff);
    Code.Insert(0xce);
    Code.Insert(0xbf);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xba);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x0f);
    Code.Insert(0x05);
    Code.Insert(0x49);
    Code.Insert(0xff);
    Code.Insert(0xcf);
    Code.Insert(0x49);
    Code.Insert(0x83);
    Code.Insert(0xff);
    Code.Insert(0x00);
    Code.Insert(0x75);
    Code.Insert(0xd7);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xe6);
    Code.Insert(0xc7);
    Code.Insert(0x04);
    Code.Insert(0x24);
    Code.Insert(0x0d);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xb8);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xbf);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0xba);
    Code.Insert(0x01);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x0f);
    Code.Insert(0x05);
    Code.Insert(0xc7);
    Code.Insert(0x04);
    Code.Insert(0x24);
    Code.Insert(0x0a);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x0f);
    Code.Insert(0x05);
    Code.Insert(0x48);
    Code.Insert(0x83);
    Code.Insert(0xc4);
    Code.Insert(0x10);
    Code.Insert(0x48);
    Code.Insert(0x89);
    Code.Insert(0xec);
    Code.Insert(0x5d);
}


void add()
{
    Code.Insert(88); // pop rax
    Code.Insert(91); // pop rbx

    Code.Insert(72); // add rax, rbx
    Code.Insert(1);
    Code.Insert(216);

    Code.Insert(80); // push rax
}

void sub()
{
    Code.Insert(0x5B); //pop rbx
    Code.Insert(0x58); //pop rax

    Code.Insert(0x48); // sub rax, rbx
    Code.Insert(0x29);
    Code.Insert(0xD8);

    Code.Insert(0x50); // push rax
}

void mul()
{
    Code.Insert(0x6A); //push 100
    Code.Insert(0x64);

    Code.Insert(0x9B); //wait

    Code.Insert(0xDB); //fninit
    Code.Insert(0xE3);

    Code.Insert(0xDB); //fild dword [rsp + 16]
    Code.Insert(0x44);
    Code.Insert(0x24);
    Code.Insert(0x10);

    Code.Insert(0xDA);  //fimul dword [rsp + 8]
    Code.Insert(0x4C);
    Code.Insert(0x24);
    Code.Insert(0x08);

    Code.Insert(0xDA); //fidiv dword [rsp]
    Code.Insert(0x34);
    Code.Insert(0x24);

    Code.Insert(0x48); //add rsp, 16
    Code.Insert(0x83);
    Code.Insert(0xC4);
    Code.Insert(0x10);

    Code.Insert(0x48); //mov qword [rsp], 0
    Code.Insert(0xC7);
    Code.Insert(0x04);
    Code.Insert(0x24);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);

    Code.Insert(0xDB); //fist dword [rsp]
    Code.Insert(0x14);
    Code.Insert(0x24);

    Code.Insert(0x4C); //movsx r11, dword [rsp]
    Code.Insert(0x63);
    Code.Insert(0x1C);
    Code.Insert(0x24);

    Code.Insert(0x4C); //mov qword [rsp], r11
    Code.Insert(0x89);
    Code.Insert(0x1C);
    Code.Insert(0x24);

}

void div() {
    Code.Insert(0x6A); //push 100
    Code.Insert(0x64);

    Code.Insert(0x9B); //wait

    Code.Insert(0xDB); //fninit
    Code.Insert(0xE3);

    Code.Insert(0xDB); //fild dword [rsp + 16]
    Code.Insert(0x44);
    Code.Insert(0x24);
    Code.Insert(0x10);

    Code.Insert(0xDA); //fild dword [rsp + 8]
    Code.Insert(0x74);
    Code.Insert(0x24);
    Code.Insert(0x08);

    Code.Insert(0xDA); // fimul dword [rsp]
    Code.Insert(0x0C);
    Code.Insert(0x24);

    Code.Insert(0xD9); //frndint
    Code.Insert(0xFC);

    Code.Insert(0x48); //add rsp, 16
    Code.Insert(0x83);
    Code.Insert(0xC4);
    Code.Insert(0x10);

    Code.Insert(0x48); //mov qword [rsp], 0
    Code.Insert(0xC7);
    Code.Insert(0x04);
    Code.Insert(0x24);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);
    Code.Insert(0x00);

    Code.Insert(0xDB); //fist dword [rsp]
    Code.Insert(0x14);
    Code.Insert(0x24);

    Code.Insert(0x4C); //movsx r11, dword [rsp]
    Code.Insert(0x63);
    Code.Insert(0x1C);
    Code.Insert(0x24);

    Code.Insert(0x4C); //mov qword [rsp], r11
    Code.Insert(0x89);
    Code.Insert(0x1C);
    Code.Insert(0x24);
}

void Make_ELF()
{
    Code.Insert(127);
    Code.Insert(69);
    Code.Insert(76);
    Code.Insert(70);
    Code.Insert(2);
    Code.Insert(1);
    Code.Insert(1);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(2);
    Code.Insert(0);
    Code.Insert(62);
    Code.Insert(0);
    Code.Insert(1);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(128);
    Code.Insert(0);
    Code.Insert(64);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(64);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(64);
    Code.Insert(0);
    Code.Insert(56);
    Code.Insert(0);
    Code.Insert(1);
    Code.Insert(0);
    Code.Insert(64);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(1);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(5);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(64);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(64);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0); //Size
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);  //Size;
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(32);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
    Code.Insert(0);
}