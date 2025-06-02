class MazeObject {
public:
    MazeObject();
    MazeObject(glm::vec3 s,glm::vec3 pos,float angle);
    void setPosition(glm::vec3 pos);
    void setScale(glm::vec3 scale);
    void setRotationAngle(float angle);
    void setNear(bool n);
    void setTaken(bool t);
    void hide();
    bool isNear();
    bool isTaken();
    glm::vec3 getPosition();
    glm::vec3 getScale();
    float getAngle();
    glm::mat4 calculateWorldMatrix();
    void setShow(bool s);
    bool isShowable();
    void startPosition(char** m,char sym,int row,int col, float s,float meas,float h);
private:
    glm::vec3 scale;
    float rotationAngle;
    bool nearObj;
    bool taken;
    glm::vec3 position;
    void positionInitializer(char** m, float s,float meas,float h);
    bool show;
};


MazeObject::MazeObject() {}

MazeObject::MazeObject(glm::vec3 s,glm::vec3 pos, float angle) {
    this->scale = s;
    this->position = pos;
    this->rotationAngle = angle;
}

void MazeObject::setPosition(glm::vec3 pos){
    this->position = pos;
}

void MazeObject::setScale(glm::vec3 s){
     this->scale = s;
    
}

void MazeObject::setRotationAngle(float angle){
    this->rotationAngle = angle;
    
}
void MazeObject::setNear(bool n){
    this->nearObj = n;
}
void MazeObject::setTaken(bool t){
    this->taken = t;
}
bool MazeObject::isNear(){
    return this->nearObj;
}
bool MazeObject::isTaken(){
    return this->taken;
}


glm::vec3 MazeObject::getPosition(){
    return this->position;
    
}

glm::vec3 MazeObject::getScale(){
    return this->scale;
}
float MazeObject::getAngle(){
    return this->rotationAngle;
}
glm::mat4 MazeObject::calculateWorldMatrix(){
    if(this->isShowable())
    {
        glm::mat4 World = glm::mat4(1.0);
        World = glm::translate(World,position);
        World *= glm::rotate(glm::mat4(1.0),glm::radians(rotationAngle),{0,1,0});
        World *= glm::scale(glm::mat4(1.0),scale);
        return World;

    }else {
        return glm::mat4(0.0f);
    }

    
}

void MazeObject::hide() {
    this->setTaken(true);
    this->setNear(false);
    this->setShow(false);
}
void MazeObject::positionInitializer(char** m,float s,float meas,float h){
    int x = floor(this->getPosition().x);
    int z = floor(this->getPosition().z);
    float xFinal = x;
    float zFinal = z;
    if(x+1 < 15 && (m[z][x + 1] == '#' || m[z][x - 1] == '#'))
        xFinal = x + meas;
    if(z+1 < 15 && (m[z + 1][x] == '#' || m[z - 1][x] == '#'))
        zFinal = z + meas;
    this->setScale(glm::vec3(s));
    this->setPosition({ xFinal,h,zFinal });
    this->setNear(false);
    this->setTaken(false);
    this->setShow(true);
}
void MazeObject::startPosition(char** m,char sym,int row,int col,float s,float meas,float h){
    int done = 0;
    int i,j;
    while(done == 0){
        i = (rand() % (row / 2)) * 2 + 1;
        j = (rand() % (col / 2)) * 2 + 1;
        if (i >= row / 2 && i != row - 1 && m[i][j] == ' ') {
            m[i][j] = sym;
            setPosition({j,h,i});
            positionInitializer(m,s,meas,h);
            done++;
        }
    }
}
bool MazeObject::isShowable(){
    return this->show;
}
void MazeObject::setShow(bool s){
    this->show = s;
}




