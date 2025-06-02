struct Vertex {
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 UV;
};
class Maze {
public:
    static void destroy(char** maze, int row);
    static void print(char** maze, int row, int col);
    static void recMaze(int row, int col, char** m, int sr, int sc);
    static void create(std::vector<Vertex>& vDef, std::vector<uint32_t>& vIdx, char** maze, int row, int col);
    static void  createRoom(std::vector<Vertex>& vPos, std::vector<uint32_t>& vIdx, int exit);
    static void createFloorCeil(std::vector<Vertex>& vPos, std::vector<uint32_t>& vIdx,int row,int col);
private:
    static void helper(std::vector<Vertex>& vPos, std::vector<uint32_t>& vIdx, int* triangleIdx, int v[24],int count);
};
void Maze::destroy(char** maze,int row){
    for (int i = 0; i < row; i++) {
        free(maze[i]);
    }
    free(maze);
}
void Maze::print(char** maze,int row, int col){
    for (int i = row-1; i >= 0; i--) {
        for (int j = col-1; j >= 0; j--) {
            std::cout << maze[i][j];
        }
        std::cout << "\n";
    }
    std::cout << "\n";

}

void Maze::create(std::vector<Vertex>& vPos, std::vector<uint32_t>& vIdx,char**maze,int row,int col) {
       int posY = 0;
       int triangleIdx = 0;
       int count = 0;
       int v[24];
       for(int i = 0;i<24;i++){
           v[i] = 0;
       }
       bool modify=false;
       
       for(int posX = 0;posX<col;posX++){
                modify = false;
        for(int posZ = 0;posZ<row;posZ++){

            if(maze[posZ][posX] == '#'){
                count ++;
                    if(modify == true){
                        v[3] =posX;
                        v[4] = posY;
                        v[5]=posZ+1; // 1
                      
                        v[6] =posX;
                        v[7] = posY+1;
                        v[8]=posZ+1; // 2
                          
                        v[15] = posX+1;
                        v[16] = posY;
                        v[17] =posZ+1; // 5
                          
                        v[18] = posX+1;
                        v[19] = posY+1;
                        v[20] = posZ+1; // 6
                    }
                    else{
                        modify = true;
                        v[0]=posX;
                        v[1] = posY;
                        v[2] = posZ; //0
                          
                        v[3] =posX;
                        v[4] = posY;
                        v[5]=posZ+1; // 1
                          
                        v[6] =posX;
                        v[7] = posY+1;
                        v[8]=posZ+1; // 2
                          
                        v[9] = posX;
                        v[10] = posY+1;
                        v[11] = posZ; // 3
                          
                        v[12] = posX+1;
                        v[13] = posY;
                        v[14] = posZ; // 4
                          
                        v[15] = posX+1;
                        v[16] = posY;
                        v[17] = posZ+1; // 5
                          
                        v[18] = posX+1;
                        v[19] = posY+1;
                        v[20] = posZ+1    ; // 6
                          
                        v[21] = posX+1;
                        v[22] = posY+1;
                        v[23] = posZ; // 7
                    }
                }
                else{
                    if(modify == true){
                        modify = false;
                        helper(vPos,vIdx,&triangleIdx,v,count);
                        count = 0;
                        triangleIdx = triangleIdx+16;

                    }
                }
        }
         
              if(modify == true){
                       modify = false;
                       helper(vPos,vIdx,&triangleIdx,v,count);
                       count = 0;
                       triangleIdx = triangleIdx+16;
                        }
                    }
}

void Maze::recMaze(int row, int col,char **maze,int sr, int sc){
    int dirs[4];

    dirs[0] = dirs[1] = dirs[2] = dirs[3] = 0;
    for (int i = 1; i <= 4; i++) {
        int p = rand() % 4;
        for (int j = 0; j < 4; j++) {
            if (dirs[(p + j) % 4] == 0) {
                dirs[(p + j) % 4] = i;
                break;
            }
        }
    }
    for (int j = 0; j < 4; j++) {
        switch (dirs[j]) {
        case 1:
            if (sr - 2 < 0) continue;
            if (maze[sr - 2][sc] == '#') {
                maze[sr - 1][sc] = ' ';
                maze[sr - 2][sc] = ' ';
                recMaze(row, col, maze, sr - 2, sc);
            }
            break;
        case 2:
            if (sr + 2 >= row) continue;
            if (maze[sr + 2][sc] == '#') {
                maze[sr + 1][sc] = ' ';
                maze[sr + 2][sc] = ' ';
                recMaze(row, col, maze, sr + 2, sc);
            }
            break;
        case 3:
            if (sc - 2 < 0) continue;
            if (maze[sr][sc - 2] == '#') {
                maze[sr][sc - 1] = ' ';
                maze[sr][sc - 2] = ' ';
                recMaze(row, col, maze, sr, sc - 2);
            }
            break;
        case 4:
            if (sc + 2 >= col) continue;
            if (maze[sr][sc + 2] == '#') {
                maze[sr][sc + 1] = ' ';
                maze[sr][sc + 2] = ' ';
                recMaze(row, col, maze, sr, sc + 2);
            }
            break;
        default:
            break;
        }
    }
}
void Maze::helper(std::vector<Vertex> &vPos, std::vector<uint32_t> &vIdx, int *triangleIdx, int v[24],int count){
    for(int i = 0;i<24;i+=3){
        switch (i) {
            case 0:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,-1.0f},{float(1.0f/15.0f) ,0.0f}});//A
                vPos.push_back({{v[i],v[i+1],v[i+2]},{-1.0f,0.0f,0.0f},{0.0f,0.0f}});//A'
                break;
            case 3:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,1.0f},{0.0f,0.0f}});//B
                vPos.push_back({{v[i],v[i+1],v[i+2]},{-1.0f,0.0f,0.0f},{float(count/15.0f),0.0f}});//B'
                break;
            case 6:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,1.0f},{0.0f,1.0f }});//C
                vPos.push_back({{v[i],v[i+1],v[i+2]},{-1.0f,0.0f,0.0f},{float(count/15.0f),1.0f }});//C'
                break;
            case 9:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,-1.0f},{float(1.0f/15.0f) ,1.0f}});//D
                vPos.push_back({{v[i],v[i+1],v[i+2]},{-1.0f,0.0f,0.0f},{0.0f,1.0f}});//D'
                break;
            case 12:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,-1.0f},{0.0f,0.0f}});//E
                vPos.push_back({{v[i],v[i+1],v[i+2]},{1.0f,0.0f,0.0f},{float(count/15.0f),0.0f}});//E'
                break;
            case 15:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,1.0f},{float(1.0f/15.0f),0.0f}});//F
                vPos.push_back({{v[i],v[i+1],v[i+2]},{1.0f,0.0f,0.0f},{0.0f,0.0f}});//F'
                break;
            case 18:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,1.0f},{float(1.0f/15.0f),1.0f}});//G
                vPos.push_back({{v[i],v[i+1],v[i+2]},{1.0f,0.0f,0.0f},{0.0f,1.0f}});//G'
                break;
            case 21:
                vPos.push_back({{v[i],v[i+1],v[i+2]},{0.0f,0.0f,-1.0f},{0.0f,1.0f}});//H
                vPos.push_back({{v[i],v[i+1],v[i+2]},{1.0f,0.0f,0.0f},{float(count/15.0f),1.0f}});//H'
                break;
                
            default:
                break;
        }
    }

        
        
        
        vIdx.push_back((*triangleIdx)+1); vIdx.push_back((*triangleIdx)+3); vIdx.push_back((*triangleIdx)+5); // 0 1 2 = A'B'C'
        vIdx.push_back((*triangleIdx)+1); vIdx.push_back((*triangleIdx)+ 5); vIdx.push_back((*triangleIdx)+7); // 0 2 3 = A'C'D'
        vIdx.push_back(*triangleIdx); vIdx.push_back((*triangleIdx)+6); vIdx.push_back((*triangleIdx)+8); // 0 3 4 = ADE
        vIdx.push_back((*triangleIdx)+6); vIdx.push_back((*triangleIdx)+8); vIdx.push_back((*triangleIdx)+14); // 3 4 7 = DEH
        vIdx.push_back((*triangleIdx)+11); vIdx.push_back((*triangleIdx)+9); vIdx.push_back((*triangleIdx)+15); // 5 7 4 = F'E'H'
        vIdx.push_back((*triangleIdx)+11); vIdx.push_back((*triangleIdx)+ 13); vIdx.push_back((*triangleIdx)+15); // 5 6 7 = F'G'H'
        vIdx.push_back((*triangleIdx)+2); vIdx.push_back((*triangleIdx)+12); vIdx.push_back((*triangleIdx)+10); // 1 6 5 = BGF
        vIdx.push_back((*triangleIdx)+2); vIdx.push_back((*triangleIdx)+ 4); vIdx.push_back((*triangleIdx)+12); // 1 2 6 = BCG
        vIdx.push_back((*triangleIdx)+4); vIdx.push_back((*triangleIdx)+ 6); vIdx.push_back((*triangleIdx)+14); // 2 3 7 = CDH
        vIdx.push_back((*triangleIdx)+4); vIdx.push_back((*triangleIdx)+ 12); vIdx.push_back((*triangleIdx)+14); // 2 6 7 = CGH
        vIdx.push_back(*triangleIdx); vIdx.push_back((*triangleIdx)+ 10); vIdx.push_back((*triangleIdx)+8); // 0 4 5 AFE
        vIdx.push_back(*triangleIdx); vIdx.push_back((*triangleIdx)+ 10); vIdx.push_back((*triangleIdx)+2); // 0 5 1 = AFB
    
    
    
    
}

void Maze::createRoom(std::vector<Vertex>& vPos, std::vector<uint32_t>& vIdx,int exit){
        
        
        vPos.push_back({ {exit,0,15},{0.0f,1.0f,0.0f},{0.293,1} });//0
        vPos.push_back({ {exit,0,15},{1.0f,0.0f,0.0f},{0.5869,0} });//0'
        vPos.push_back({ {exit,0,15},{0.0f,0.0f,1.0f},{-1,-1} });//0''

        vPos.push_back({ {exit,0,20},{0.0f,1.0f,0.0f},{0.293,0} });//1
        vPos.push_back({ {exit,0,20},{1.0f,0.0f,0.0f},{0.5869,1} });//1'
        vPos.push_back({ {exit,0,20},{0.0f,0.0f,-1.0f},{1,0.778} });//1''

        vPos.push_back({ {exit,1,20},{0.0f,-1.0f,0.0f},{1,0.1 } });//2
        vPos.push_back({ {exit,1,20},{1.0f,0.0f,0.0f},{0.293,1} });//2'
        vPos.push_back({ {exit,1,20},{0.0f,0.0f,-1.0f},{1,0.22} });//2''

        vPos.push_back({ {exit,1,15},{0.0f,-1.0f,0.0f},{0.6,0.1 } });//3
        vPos.push_back({ {exit,1,15},{1.0f,0.0f,0.0f},{0.293,0 } });//3'
        vPos.push_back({ {exit,1,15},{0.0f,0.0f,1.0f},{-1,-1 } });//3''

        vPos.push_back({ {exit+1,0,15},{0.0f,1.0f,0.0f},{0,1 } });//4
        vPos.push_back({ {exit+1,0,15},{-1.0f,0.0f,0.0f},{0.5869,0 } });//4'
        vPos.push_back({ {exit+1,0,15},{0.0f,0.0f,1.0f},{-1,-1 } });//4''

        vPos.push_back({ {exit+1,0,20},{0.0f,1.0f,0.0f},{0,0 } });//5
        vPos.push_back({ {exit+1,0,20},{-1.0f,0.0f,0.0f},{0.5869,1 } });//5'
        vPos.push_back({ {exit+1,0,20},{0.0f,0.0f,-1.0f},{0.5869,0.778 } });//5''
        
        vPos.push_back({ {exit+1,1,20},{0.0f,-1.0f,0.0f},{1,0 } });//6
        vPos.push_back({ {exit+1,1,20},{-1.0f,0.0f,0.0f},{0.293,1 } });//6'
        vPos.push_back({ {exit+1,1,20},{0.0f,0.0f,-1.0f},{0.5869,0.22 } });//6''

        vPos.push_back({ {exit + 1,1,15},{0.0f,-1.0f,0.0f},{0.6,0} });//7
        vPos.push_back({ {exit + 1,1,15},{-1.0f,0.0f,0.0f},{0.293,0 } });//7'
        vPos.push_back({ {exit + 1,1,15},{0.0f,0.0f,1.0f},{-1,-1 } });//7''

        vIdx.push_back(0*3+1);vIdx.push_back(1*3+1);vIdx.push_back(2*3+1);
        vIdx.push_back(0*3+1); vIdx.push_back(2*3+1); vIdx.push_back(3*3+1);

        vIdx.push_back(5*3+1); vIdx.push_back(6*3+1); vIdx.push_back(7*3+1);
        vIdx.push_back(7*3+1);vIdx.push_back(4*3+1); vIdx.push_back(5*3+1);

        vIdx.push_back(1*3+2); vIdx.push_back(6*3+2); vIdx.push_back(5*3+2);
        vIdx.push_back(1*3+2); vIdx.push_back(2*3+2); vIdx.push_back(6*3+2);
        
        vIdx.push_back(7*3); vIdx.push_back(6*3); vIdx.push_back(2*3);
        vIdx.push_back(3*3);vIdx.push_back(2*3);vIdx.push_back(7*3);
        
        vIdx.push_back(4*3); vIdx.push_back(5*3); vIdx.push_back(0*3);
        vIdx.push_back(1*3);vIdx.push_back(0*3); vIdx.push_back(5*3);
        
    }

void Maze::createFloorCeil(std::vector<Vertex>& vPos, std::vector<uint32_t>& vIdx,int row,int col)
{
    //FLOOR
    vPos.push_back({{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{1,1}}); // vertex 0
    vPos.push_back({{col,0.0f,0.0f},{0.0f,1.0f,0.0f},{0,1}}); // vertex 1
    vPos.push_back({{0.0f,0.0f,row},{0.0f,1.0f,0.0f},{1,0}}); // vertex 2
    vPos.push_back({{col,0.0f,row},{0.0f,1.0f,0.0f},{0,0}}); // vertex 3
    
     vIdx.push_back(0); vIdx.push_back(1); vIdx.push_back(2); // First triangle
    vIdx.push_back(1); vIdx.push_back(2); vIdx.push_back(3); // Second triangle
    //CEIL
    vPos.push_back({{0.0f,1.0f,0.0f},{0.0f,-1.0f,0.0f},{1,1}}); // vertex 0
    vPos.push_back({{col,1.0f,0.0f},{0.0f,-1.0f,0.0f},{0,1}}); // vertex 1
    vPos.push_back({{0.0f,1.0f,row},{0.0f,-1.0f,0.0f},{1,0}}); // vertex 2
    vPos.push_back({{col,1.0f,row},{0.0f,-1.0f,0.0f},{0,0}}); // vertex 3
    
     vIdx.push_back(4); vIdx.push_back(5); vIdx.push_back(6); // First triangle
    vIdx.push_back(5); vIdx.push_back(6); vIdx.push_back(7); // Second triangle
}
