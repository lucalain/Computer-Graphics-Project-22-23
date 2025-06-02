#include "Starter.hpp"
#include "Maze.hpp"
#include "MazeObject.hpp"

#define N 5
//N is the number of instances of each decoration
struct UniformBufferObject {
    alignas(16) glm::mat4 mvpMat;
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};

struct UniformBufferObjectD {
    alignas(16) glm::mat4 mvpMat[5];
    alignas(16) glm::mat4 mMat;
    alignas(16) glm::mat4 nMat;
};


struct GlobalUniformBufferObject {
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};
struct GlobalUniformBufferObjectR {
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};
struct GlobalUniformBufferObjectD {
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec4 lightColor;
    alignas(16) glm::vec3 eyePos;
};

struct OverlayUniformBlock {
    alignas(4) float visible;
};


struct VertexOverlay {
    glm::vec2 pos;
    glm::vec2 UV;
};
struct VertexObject{
    glm::vec3 pos;
    glm::vec2 UV;
};



// MAIN !
class MeshLoader : public BaseProject {
protected:

    // Current aspect ratio (used by the callback that resized the window
    float Ar;
    // Descriptor Layouts ["classes" of what will be passed to the shaders]
    DescriptorSetLayout DSL, DSLO;

    // Vertex formats
    VertexDescriptor VD, VOverlay,VDObject;

    // Pipelines [Shader couples]
    Pipeline P, POverlay,PMaze,PRoom,PDecoration;

    // Models, textures and Descriptors (values assigned to the uniforms)
    // Please note that Model objects depends on the corresponding vertex structure
    // Models
    Model<Vertex> MMaze;
    Model<Vertex> MRoom,MFloorCeil;
    Model<Vertex> MCrystal,MSkull;
    Model<VertexObject> MDoor,MKeys[2],MEnemy,MHeart,MShield;

    
    Model<VertexOverlay> MTake, MStart, MUse,MSymbolHeart[3],MPick,MGet,MProtection,MSymbolKey[2];
    //Pick -> Heart -> life      Get -> Shield -> protection
    Model<VertexOverlay> MWin, MLose;
    // Descriptor sets
    DescriptorSet DSMaze, DSKeys[2], DSDoor, DSTake, DSStart, DSUse,DSEnemy,DSSymbolHeart[3],DSHeart,DSPick,
        DSShield,DSGet,DSProtection,DSRoom,DSSymbolKey[2],DSFloorCeil,DSWin,DSLose;
    DescriptorSet DSCrystal,DSSkull;
    // Textures
    Texture TMaze, TKeys, TDoor, TTake, TStart, TUse,TEnemy,TSymbolHeart,THeart,TPick,TShield,TGet,TProtection,TRoom,TSymbolKey,TFloorCeil,TLose,TWin;
    Texture TDecoration;

    // C++ storage for uniform variables
    UniformBufferObject ubo,uboMaze,uboRoom;
    UniformBufferObjectD uboDecoration;
    
    GlobalUniformBufferObject guboMaze;
    GlobalUniformBufferObjectR guboRoom;
    GlobalUniformBufferObjectD guboDecoration;
        
    OverlayUniformBlock oub;
    // Other application parameters
    MazeObject keys[2];
    MazeObject door,heart;
    MazeObject shield;
    MazeObject crystal[N];
    MazeObject skull[N];

    glm::vec3 CamPos;
    float CamAlpha = 180.0f;
    float CamBeta = 0.0f;
    int exit, gameState;
    char** maze;
    glm::vec3 keySpawnPos;
    glm::vec3 camPosValidPosition;
    float xSaved, zSaved;
    
    glm::quat KeyRot = glm::quat(glm::vec3(0, glm::radians(32.7f), 0)) *
        glm::quat(glm::vec3(glm::radians(25.2f), 0, 0)) *
        glm::quat(glm::vec3(0, 0, glm::radians(-82.f)));


    bool MoveCam = true,
    used = false;
    
    glm::vec3 posEnemy;
    float xSavedE, zSavedE;
    glm::vec3 posEnemyValid;
    int enter;
    
    int lives = 2; // number of lives
    bool mazeDestroyed = false;
    
    bool shieldUsed = false,open = false,win=false;

    // Here you set the main application parameters
    void setWindowParameters() {
        // window size, titile and initial background
        windowWidth = 800;
        windowHeight = 600;
        windowTitle = "Computer Graphics";
        windowResizable = GLFW_TRUE;
        initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

        // Descriptor pool sizes
        uniformBlocksInPool = 29;
        texturesInPool = 24;
        setsInPool = 24;

        Ar = (float)windowWidth / (float)windowHeight;
    }

    // What to do when the window changes size
    void onWindowResize(int w, int h) {
        Ar = (float)w / (float)h;
    }

    // Here you load and setup all your Vulkan Models and Texutures.
    // Here you also create your Descriptor set layouts and load the shaders for the pipelines
    void localInit() {

        //initialization of MazeObjects
        keys[0] = MazeObject();
        keys[1] = MazeObject();
        door = MazeObject();
        heart = MazeObject();
        shield = MazeObject();
        for(int i = 0;i<N;i++)
        {
            crystal[i] = MazeObject();
            skull[i] = MazeObject();
        }
        // Descriptor Layouts [what will be passed to the shaders]
        DSL.init(this, {
            // this array contains the bindings:
            // first  element : the binding number
            // second element : the type of element (buffer or texture)
            //                  using the corresponding Vulkan constant
            // third  element : the pipeline stage where it will be used
            //                  using the corresponding Vulkan constant
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
            });

  
        DSLO.init(this, {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
                    {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
            });
        // Vertex descriptors
        VD.init(this, {
            // this array contains the bindings
            // first  element : the binding number
            // second element : the stride of this binging
            // third  element : whether this parameter change per vertex or per instance
            //                  using the corresponding Vulkan constant
            {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
                // this array contains the location
                // first  element : the binding number
                // second element : the location number
                // third  element : the offset of this element in the memory record
                // fourth element : the data type of the element
                //                  using the corresponding Vulkan constant
                // fifth  elmenet : the size in byte of the element
                // sixth  element : a constant defining the element usage
                //                   POSITION - a vec3 with the position
                //                   NORMAL   - a vec3 with the normal vector
                //                   UV       - a vec2 with a UV coordinate
                //                   COLOR    - a vec4 with a RGBA color
                //                   TANGENT  - a vec4 with the tangent vector
                //                   OTHER    - anything else
                //
                // ***************** DOUBLE CHECK ********************
                //    That the Vertex data structure you use in the "offsetoff" and
                //    in the "sizeof" in the previous array, refers to the correct one,
                //    if you have more than one vertex format!
                // ***************************************************
              {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
                       sizeof(glm::vec3), POSITION},
              {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
                  sizeof(glm::vec3), NORMAL},
              {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
                       sizeof(glm::vec2), UV}
            });
      
        VOverlay.init(this, {
                  {0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
              {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
                     sizeof(glm::vec2), OTHER},
              {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
                     sizeof(glm::vec2), UV}
            });
        VDObject.init(this, {
                  {0, sizeof(VertexObject), VK_VERTEX_INPUT_RATE_VERTEX}
            }, {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexObject, pos),
                         sizeof(glm::vec3), POSITION},
                {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexObject, UV),
                     sizeof(glm::vec2), UV}
            });

        // Pipelines [Shader couples]
        // The second parameter is the pointer to the vertex definition
        // Third and fourth parameters are respectively the vertex and fragment shaders
        // The last array, is a vector of pointer to the layouts of the sets that will
        // be used in this pipeline. The first element will be set 0, and so on..
        P.init(this, &VDObject, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", { &DSLO });
        P.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        
        PRoom.init(this, &VD, "shaders/ShaderMazeVert.spv", "shaders/ShaderRoomFrag.spv", { &DSL });
        PRoom.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        
        PDecoration.init(this, &VD, "shaders/ShaderDecorationVert.spv", "shaders/ShaderDecorationFrag.spv", { &DSL });
        PDecoration.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        
        PMaze.init(this, &VD, "shaders/ShaderMazeVert.spv", "shaders/ShaderMazeFrag.spv", { &DSL });
        PMaze.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
        
        POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", { &DSLO });
        POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
     

        //creation of Maze
        int r = 15;
        int c = 15;

        maze = genMaze(r, c);

        Maze::create(MMaze.vertices, MMaze.indices, maze,r,c);
        Maze::print(maze, r, c);
        Maze::createRoom(MRoom.vertices,MRoom.indices,exit);
        Maze::createFloorCeil(MFloorCeil.vertices,MFloorCeil.indices, r, c);


        // Models, textures and Descriptors (values assigned to the uniforms)

        // Create models
        // The second parameter is the pointer to the vertex definition for this model
        // The third parameter is the file name
        // The last is a constant specifying the file type: currently only OBJ or GLTF

        MMaze.initMesh(this, &VD);
        MRoom.initMesh(this, &VD);
        MFloorCeil.initMesh(this, &VD);
        
        MKeys[0].init(this, &VDObject, "Models/Key.obj", OBJ);
        MKeys[1].init(this, &VDObject, "Models/Key.obj", OBJ);
        MDoor.init(this, &VDObject, "Models/Door.obj", OBJ);
        MEnemy.init(this, &VDObject, "Models/Enemy.obj", OBJ);
        MHeart.init(this,&VDObject,"Models/MedicalBox.obj",OBJ);
        MShield.init(this,&VDObject,"Models/Shield.obj",OBJ);
      
        MCrystal.init(this,&VD,"Models/decoration.013_Mesh.4440.mgcg",MGCG);
        MSkull.init(this,&VD,"Models/bones.029_Mesh.5843.mgcg",MGCG);
        
        
        // Creates a mesh with direct enumeration of vertices and indices
        MTake.vertices = { {{-0.8f, 0.6f}, {0.0f,0.0f}}, {{-0.8f, 0.95f}, {0.0f,1.0f}},
                         {{ 0.8f, 0.6f}, {1.0f,0.0f}}, {{ 0.8f, 0.95f}, {1.0f,1.0f}} };
        MTake.indices = { 0, 1, 2,    1, 2, 3 };
        MTake.initMesh(this, &VOverlay);

        MUse.vertices = { {{-0.8f, 0.6f}, {0.0f,0.0f}}, {{-0.8f, 0.95f}, {0.0f,1.0f}},
                         {{ 0.8f, 0.6f}, {1.0f,0.0f}}, {{ 0.8f, 0.95f}, {1.0f,1.0f}} };
        MUse.indices = { 0, 1, 2,    1, 2, 3 };
        MUse.initMesh(this, &VOverlay);

        MStart.vertices = { {{-1.0f, -0.58559f}, {0.0102f, 0.0f}}, {{-1.0f, 0.58559f}, {0.0102f,0.85512f}},
                         {{ 1.0f,-0.58559f}, {1.0f,0.0f}}, {{ 1.0f, 0.58559f}, {1.0f,0.85512f}} };
        MStart.indices = { 0, 1, 2,    1, 2, 3 };
        MStart.initMesh(this, &VOverlay);
        
        MSymbolHeart[0].vertices = { {{0.9f, -0.9f}, {0.0f,0.0f}}, {{0.9f, -0.8f}, {0.0f,1.0f}},
            {{ 1.0f,-0.8f}, {1.0f,1.0f}}, {{1.0f,-0.9f}, {1.0f,0.0f}} };
        MSymbolHeart[0].indices = { 0, 1, 2,    0, 2, 3 };
        MSymbolHeart[0].initMesh(this, &VOverlay);
        
        MSymbolHeart[1].vertices = { {{0.8f, -0.9f}, {0.0f,0.0f}}, {{0.8f, -0.8f}, {0.0f,1.0f}},
            {{ 0.9f,-0.8f}, {1.0f,1.0f}}, {{0.9f,-0.9f}, {1.0f,0.0f}} };
        MSymbolHeart[1].indices = { 0, 1, 2,    0, 2, 3 };
        MSymbolHeart[1].initMesh(this, &VOverlay);
        
        MSymbolHeart[2].vertices = { {{0.7f, -0.9f}, {0.0f,0.0f}}, {{0.7f, -0.8f}, {0.0f,1.0f}},
            {{ 0.8f,-0.8f}, {1.0f,1.0f}}, {{0.8f,-0.9f}, {1.0f,0.0f}} };
        MSymbolHeart[2].indices = { 0, 1, 2,    0, 2, 3 };
        MSymbolHeart[2].initMesh(this, &VOverlay);
        
        MProtection.vertices = { {{0.6f, -0.9f}, {0.0f,0.0f}}, {{0.6f, -0.8f}, {0.0f,1.0f}},
            {{ 0.7f,-0.8f}, {1.0f,1.0f}}, {{0.7f,-0.9f}, {1.0f,0.0f}} };
        MProtection.indices = { 0, 1, 2,    0, 2, 3 };
        MProtection.initMesh(this, &VOverlay);
        
        MSymbolKey[0].vertices = { {{-0.9f, -0.95f}, {0.0f,0.0f}}, {{-0.9f, -0.75f}, {0.0f,1.0f}},
            {{ -0.8f,-0.75f}, {1.0f,1.0f}}, {{-0.8f,-0.95f}, {1.0f,0.0f}} };
        MSymbolKey[0].indices = { 0, 1, 2,    0, 2, 3 };
        MSymbolKey[0].initMesh(this, &VOverlay);
        
        MSymbolKey[1].vertices = { {{-0.8f, -0.95f}, {0.0f,0.0f}}, {{-0.8f, -0.75f}, {0.0f,1.0f}},
            {{ -0.7f,-0.75f}, {1.0f,1.0f}}, {{-0.7f,-0.95f}, {1.0f,0.0f}} };
        MSymbolKey[1].indices = { 0, 1, 2,    0, 2, 3 };
        MSymbolKey[1].initMesh(this, &VOverlay);
        
        MPick.vertices = { {{-0.8f, 0.6f}, {0.0f,0.0f}}, {{-0.8f, 0.95f}, {0.0f,1.0f}},
                         {{ 0.8f, 0.6f}, {1.0f,0.0f}}, {{ 0.8f, 0.95f}, {1.0f,1.0f}} };
        MPick.indices = { 0, 1, 2,    1, 2, 3 };
        MPick.initMesh(this, &VOverlay);
        
        MGet.vertices = { {{-0.8f, 0.6f}, {0.0f,0.0f}}, {{-0.8f, 0.95f}, {0.0f,1.0f}},
                         {{ 0.8f, 0.6f}, {1.0f,0.0f}}, {{ 0.8f, 0.95f}, {1.0f,1.0f}} };
        MGet.indices = { 0, 1, 2,    1, 2, 3 };
        MGet.initMesh(this, &VOverlay);
        
        MWin.vertices = { {{-1.0f, -0.7f}, {0.0f, 0.0f}}, {{-1.0f, 0.7f}, {0.0f,1.0f}},
                         {{ 1.0f,-0.7f}, {1.0f,0.0f}}, {{ 1.0f, 0.7f}, {1.0f,1.0f}} };
        MWin.indices = { 0, 1, 2,    1, 2, 3 };
        MWin.initMesh(this, &VOverlay);
        
        MLose.vertices = { {{-1.0f, -0.7f}, {0.0f, 0.0f}}, {{-1.0f, 0.7f}, {0.0f,1.0f}},
            {{ 1.0f,-0.7f}, {1.0f,0.0f}}, {{ 1.0f, 0.7f}, {1.0f,1.0f}} };
        MLose.indices = { 0, 1, 2,    1, 2, 3 };
        MLose.initMesh(this, &VOverlay);
        

        // Create the textures
        // The second parameter is the file name
        TMaze.init(this, "textures/Wall.png");
        TKeys.init(this, "textures/Key.png");
        TDoor.init(this, "textures/Gold.png");
        TTake.init(this, "textures/OverlayKey.png");
        TStart.init(this, "textures/Start.png");
        TUse.init(this, "textures/OverlayDoor.png");
        TEnemy.init(this, "textures/Enemy.jpeg");
        TSymbolHeart.init(this, "textures/Life.png");
        THeart.init(this,"textures/MedicalBox.png");
        TPick.init(this, "textures/OverlayHeart.png");
        TShield.init(this, "textures/Shield.png");
        TGet.init(this,"textures/OverlayShield.png");
        TProtection.init(this,"textures/SymbolShield.png");
        TRoom.init(this, "textures/Room.png");
        TSymbolKey.init(this,"textures/SymbolKey.png");
        TFloorCeil.init(this, "textures/FloorCeil.jpg");
        TLose.init(this,"textures/Defeat.png");
        TDecoration.init(this,"textures/Dungeon.png");
        TWin.init(this,"textures/Victory.png");




        // Init local variables
        gameState = 0;
        
    }


    // Here you create your pipelines and Descriptor Sets!
    void pipelinesAndDescriptorSetsInit() {
        
        // This creates a new pipeline (with the current surface), using its shaders
        P.create();
        POverlay.create();
        PMaze.create();
        PRoom.create();
        PDecoration.create();
        // Here you define the data set
        DSMaze.init(this, &DSL, {
            // the second parameter, is a pointer to the Uniform Set Layout of this set
            // the last parameter is an array, with one element per binding of the set.
            // first  elmenet : the binding number
            // second element : UNIFORM or TEXTURE (an enum) depending on the type
            // third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
            // fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, UNIFORM,sizeof(GlobalUniformBufferObject),nullptr},
                        {2, TEXTURE, 0, &TMaze}
            });
        DSEnemy.init(this, &DSLO, {
                        {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                        {1, TEXTURE, 0, &TEnemy}
            });
        DSKeys[0].init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TKeys}
            });
        DSKeys[1].init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TKeys}
            });
        DSDoor.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TDoor}
            });
        DSRoom.init(this, &DSL, {
                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                            {1, UNIFORM,sizeof(GlobalUniformBufferObjectR),nullptr},
                            {2, TEXTURE, 0, &TRoom}
                    });
        DSHeart.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &THeart}
            });
        DSShield.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TShield}
            });
        DSFloorCeil.init(this, &DSL, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, UNIFORM,sizeof(GlobalUniformBufferObject),nullptr},
                    {2, TEXTURE, 0, &TFloorCeil}
            });
      
        DSCrystal.init(this, &DSL, {
                                {0, UNIFORM, sizeof(UniformBufferObjectD), nullptr},
                                {1, UNIFORM,sizeof(GlobalUniformBufferObjectD),nullptr},
                                {2, TEXTURE, 0, &TDecoration}
                        });
        DSSkull.init(this, &DSL, {
                                {0, UNIFORM, sizeof(UniformBufferObjectD), nullptr},
                                {1, UNIFORM,sizeof(GlobalUniformBufferObjectD),nullptr},
                                {2, TEXTURE, 0, &TDecoration}
                        });
        
        
        DSTake.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                    {1, TEXTURE, 0, &TTake}
            });
        DSStart.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                    {1, TEXTURE, 0, &TStart}
            });
        DSUse.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                    {1, TEXTURE, 0, &TUse}
            });
        for(int i = 0; i< 2;i++)
        {
            DSSymbolKey[i].init(this, &DSLO, {
                        {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                        {1, TEXTURE, 0, &TSymbolKey}
                });
        }
        
        for(int i = 0;i< 3;i++){
            DSSymbolHeart[i].init(this, &DSLO, {
                {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                {1, TEXTURE, 0, &TSymbolHeart}
            });
        }
     
        DSPick.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                    {1, TEXTURE, 0, &TPick}
            });
        DSGet.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                    {1, TEXTURE, 0, &TGet}
            });
        DSProtection.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
                    {1, TEXTURE, 0, &TProtection}
            });
        DSWin.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TWin}
            });
        DSLose.init(this, &DSLO, {
                    {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                    {1, TEXTURE, 0, &TLose}
            });
    }

    // Here you destroy your pipelines and Descriptor Sets!
    // All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
    void pipelinesAndDescriptorSetsCleanup() {
        // Cleanup pipelines
        P.cleanup();
        POverlay.cleanup();
        PMaze.cleanup();
        PRoom.cleanup();
        PDecoration.cleanup();
        // Cleanup datasets
        DSMaze.cleanup();
        DSKeys[0].cleanup();
        DSKeys[1].cleanup();
        DSDoor.cleanup();
        DSTake.cleanup();
        DSStart.cleanup();
        DSUse.cleanup();
        DSEnemy.cleanup();
        DSSymbolHeart[0].cleanup();
        DSSymbolHeart[1].cleanup();
        DSSymbolHeart[2].cleanup();
        DSHeart.cleanup();
        DSPick.cleanup();
        DSShield.cleanup();
        DSGet.cleanup();
        DSProtection.cleanup();
        DSRoom.cleanup();
        DSSymbolKey[0].cleanup();
        DSSymbolKey[1].cleanup();
        DSFloorCeil.cleanup();
        DSWin.cleanup();
        DSLose.cleanup();

        DSCrystal.cleanup();
        DSSkull.cleanup();
        


    }

    // Here you destroy all the Models, Texture and Desc. Set Layouts you created!
    // All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
    // You also have to destroy the pipelines: since they need to be rebuilt, they have two different
    // methods: .cleanup() recreates them, while .destroy() delete them completely
    void localCleanup() {
        // Cleanup textures
        TMaze.cleanup();
        TKeys.cleanup();
        TDoor.cleanup();
        TEnemy.cleanup();
        TTake.cleanup();
        TStart.cleanup();
        TUse.cleanup();
        TSymbolHeart.cleanup();
        THeart.cleanup();
        TPick.cleanup();
        TShield.cleanup();
        TGet.cleanup();
        TProtection.cleanup();
        TRoom.cleanup();
        TSymbolKey.cleanup();
        TFloorCeil.cleanup();
        TWin.cleanup();
        TLose.cleanup();
        TDecoration.cleanup();

        // Cleanup models
        MMaze.cleanup();
        MKeys[0].cleanup();
        MKeys[1].cleanup();
        MDoor.cleanup();
        MTake.cleanup();
        MStart.cleanup();
        MUse.cleanup();
        MEnemy.cleanup();
        MSymbolHeart[0].cleanup();
        MSymbolHeart[1].cleanup();
        MSymbolHeart[2].cleanup();
        MHeart.cleanup();
        MPick.cleanup();
        MShield.cleanup();
        MGet.cleanup();
        MProtection.cleanup();
        MRoom.cleanup();
        MSymbolKey[0].cleanup();
        MSymbolKey[1].cleanup();
        MFloorCeil.cleanup();
        MWin.cleanup();
        MLose.cleanup();
    
        MCrystal.cleanup();
        MSkull.cleanup();

        // Cleanup descriptor set layouts
        DSL.cleanup();
        DSLO.cleanup();
        

        // Destroies the pipelines
        POverlay.destroy();
        P.destroy();
        PMaze.destroy();
        PRoom.destroy();
        PDecoration.destroy();

    }

    // Here it is the creation of the command buffer:
    // You send to the GPU all the objects you want to draw,
    // with their buffers and textures

    void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
        
        // binds the pipeline
        PMaze.bind(commandBuffer);

        // For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

        // binds the data set
        DSMaze.bind(commandBuffer, PMaze, 0, currentImage);
        // For a Dataset object, this command binds the corresponing dataset
        // to the command buffer and pipeline passed in its first and second parameters.
        // The third parameter is the number of the set being bound
        // As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
        // This is done automatically in file Starter.hpp, however the command here needs also the index
        // of the current image in the swap chain, passed in its last parameter

        // binds the model
        MMaze.bind(commandBuffer);
        // For a Model object, this command binds the corresponing index and vertex buffer
        // to the command buffer passed in its parameter

        // record the drawing command in the command buffer
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MMaze.indices.size()), 1, 0, 0, 0);
        
        DSFloorCeil.bind(commandBuffer, PMaze, 0, currentImage);
        MFloorCeil.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MFloorCeil.indices.size()), 1, 0, 0, 0);
        

   

        // the second parameter is the number of indexes to be drawn. For a Model object,
        // this can be retrieved with the .indices.size() method.
   
        P.bind(commandBuffer);
        
        DSKeys[0].bind(commandBuffer, P, 0, currentImage);
        MKeys[0].bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MKeys[0].indices.size()), 1, 0, 0, 0);

        DSKeys[1].bind(commandBuffer, P, 0, currentImage);
        MKeys[1].bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MKeys[1].indices.size()), 1, 0, 0, 0);
        
        
        DSDoor.bind(commandBuffer, P, 0, currentImage);
        MDoor.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MDoor.indices.size()), 1, 0, 0, 0);
        
        DSEnemy.bind(commandBuffer, P, 0, currentImage);
        MEnemy.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MEnemy.indices.size()), 1, 0, 0, 0);
        
        DSHeart.bind(commandBuffer, P, 0, currentImage);
        MHeart.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MHeart.indices.size()), 1, 0, 0, 0);
        
        DSShield.bind(commandBuffer, P, 0, currentImage);
        MShield.bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MShield.indices.size()), 1, 0, 0, 0);
        
        
        PRoom.bind(commandBuffer);
        DSRoom.bind(commandBuffer, PRoom, 0, currentImage);
                MRoom.bind(commandBuffer);
                vkCmdDrawIndexed(commandBuffer,
                    static_cast<uint32_t>(MRoom.indices.size()), 1, 0, 0, 0);
        
        POverlay.bind(commandBuffer);
        MTake.bind(commandBuffer);
        DSTake.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MTake.indices.size()), 1, 0, 0, 0);
        MStart.bind(commandBuffer);
        DSStart.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MTake.indices.size()), 1, 0, 0, 0);

        MUse.bind(commandBuffer);
        DSUse.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MUse.indices.size()), 1, 0, 0, 0);
        for(int i = 0;i<3;i++){
            MSymbolHeart[i].bind(commandBuffer);
            DSSymbolHeart[i].bind(commandBuffer, POverlay, 0, currentImage);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(MSymbolHeart[i].indices.size()), 1, 0, 0, 0);
        }
        MPick.bind(commandBuffer);
        MPick.bind(commandBuffer);
        DSPick.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MPick.indices.size()), 1, 0, 0, 0);
        
        MGet.bind(commandBuffer);
        DSGet.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MGet.indices.size()), 1, 0, 0, 0);
        
        MProtection.bind(commandBuffer);
        DSProtection.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MProtection.indices.size()), 1, 0, 0, 0);
        for(int i=0 ; i< 2;i++)
        {
            MSymbolKey[i].bind(commandBuffer);
            DSSymbolKey[i].bind(commandBuffer, POverlay, 0, currentImage);
            vkCmdDrawIndexed(commandBuffer,
                static_cast<uint32_t>(MSymbolKey[i].indices.size()), 1, 0, 0, 0);
        }
        
        MWin.bind(commandBuffer);
        DSWin.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MWin.indices.size()), 1, 0, 0, 0);
        
        MLose.bind(commandBuffer);
        DSLose.bind(commandBuffer, POverlay, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MLose.indices.size()), 1, 0, 0, 0);
        
        PDecoration.bind(commandBuffer);
        
        MCrystal.bind(commandBuffer);
        DSCrystal.bind(commandBuffer, PDecoration, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MCrystal.indices.size()), 5, 0, 0, 0);
        
       
        MSkull.bind(commandBuffer);
        DSSkull.bind(commandBuffer, PDecoration, 0, currentImage);
        vkCmdDrawIndexed(commandBuffer,
            static_cast<uint32_t>(MSkull.indices.size()), 5 , 0, 0, 0);
        
    }

    // Here is where you update the uniforms.
    // Very likely this will be where you will be writing the logic of your application.
    void updateUniformBuffer(uint32_t currentImage) {
        
  
        
        
        
        // Standard procedure to quit when the ESC key is pressed
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // Integration with the timers and the controllers
        float deltaT;
        glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
        bool fire = false;
        static float angle_animation = 0.0f;
        const float scaleEnemy = 0.2f;

        getSixAxis(deltaT, m, r, fire);
        angle_animation += (100 * deltaT);
        angle_animation = angle_animation > 360.0f ? angle_animation - 360.0f : angle_animation;

        // getSixAxis() is defined in Starter.hpp in the base class.
        // It fills the float point variable passed in its first parameter with the time
        // since the last call to the procedure.
        // It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
        // to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
        // It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
        // to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
        // If fills the last boolean variable with true if fire has been pressed:
        //          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

        // To debounce the pressing of the fire button, and start the event when the key is released
        static bool wasFire = false;
        bool handleFire = (wasFire && (!fire));
        int idxKeyNearest;
        const float deltaPos = 0.5;
        wasFire = fire;
        const float ROT_SPEED = glm::radians(120.0f);
        const float MOVE_SPEED = 2.0f;
        const float KEY_SPEED = 0.5f;
        
        static float door_animation_angle = 0.0f;
        static float debounce = false;
        static int curDebounce = 0;
        bool doorIsOpen=false;
        // Camera FOV-y, Near Plane and Far Plane
        const float FOVy = glm::radians(70.0f);
        const float nearPlane = 0.1f;
        const float farPlane = 100.0f;

        glm::mat4 M = glm::perspective(FOVy, Ar, nearPlane, farPlane);
        M[1][1] *= -1;

        glm::mat4 Mv = glm::rotate(glm::mat4(1.0), -CamBeta, glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0), -CamAlpha, glm::vec3(0, 1, 0)) *
            glm::translate(glm::mat4(1.0), -CamPos);

        glm::mat4 ViewPrj = M * Mv;

        glm::mat4 baseTr = glm::mat4(1.0f);
        
        
        
        //POINT LIGHT
        guboRoom.lightPos = glm::vec3(exit+0.5f, 0.5f, 19.0f);
        guboRoom.lightColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
        guboRoom.eyePos = CamPos;
        
        //SPOT LIGHT
        float dang = -CamBeta+ glm::radians(10.0f);

        guboMaze.lightPos = CamPos;
        guboMaze.lightDir = glm::vec3(cos(dang) * sin(CamAlpha), sin(dang), cos(dang) * cos(CamAlpha));
        guboMaze.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        guboMaze.eyePos = CamPos;
                
        glm::mat4 World;
        
        if(!open){
            ubo.mMat = baseTr;
            ubo.mvpMat = ViewPrj * door.calculateWorldMatrix();
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            DSDoor.map(currentImage, &ubo, sizeof(ubo), 0);
        }
        
        if(lives>=0 && MoveCam && !used){
            moveEnemy(deltaT);
        }
       
        World = glm::mat4(1.0f);
        World *= glm::translate(glm::mat4(1.0f), posEnemy);
        World *= glm::scale(glm::mat4(1.0f), glm::vec3(scaleEnemy));
        
       
        
        ubo.mMat = baseTr;
        ubo.mvpMat = ViewPrj * World;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        DSEnemy.map(currentImage, &ubo, sizeof(ubo), 0);
        
     

        if (MoveCam) {
            if(lives>=0 && !win){
                CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y;
                CamBeta = CamBeta - ROT_SPEED * deltaT * r.x;
                CamBeta = CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
                (CamBeta > glm::radians(90.0f) ? glm::radians(90.0f) : CamBeta);
                
                glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
                glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);
                CamPos = CamPos + MOVE_SPEED * m.x * ux * deltaT;
                
                CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0, 1, 0) * deltaT;
                
                CamPos = CamPos + MOVE_SPEED * m.z * uz * deltaT;
                
                correctPosition(CamPos,camPosValidPosition,&xSaved,&zSaved,0.2f,0.8f,false);
                if(abs(posEnemy.x-CamPos.x)<0.2 && abs(posEnemy.z-CamPos.z)<0.2){
                    if(!shield.isTaken()){
                        setCamPos(enter);
                        lives --;
                    }
                    else{
                        shieldUsed = true;
                    }
                }
                else{
                    if(shieldUsed){
                        shieldUsed = false;
                        shield.setTaken(false);
                    }
                }
                if (((CamPos.x < heart.getPosition().x + deltaPos  && CamPos.x > heart.getPosition().x - deltaPos ) &&
                    (CamPos.z < heart.getPosition().z + deltaPos  && CamPos.z > heart.getPosition().z - deltaPos )))
                {
                    heart.setNear(true);
                }
                else heart.setNear(false);
                
                if (((CamPos.x < shield.getPosition().x + deltaPos && CamPos.x > shield.getPosition().x - deltaPos ) &&
                    (CamPos.z < shield.getPosition().z + deltaPos && CamPos.z > shield.getPosition().z - deltaPos)))
                {
                    shield.setNear(true);
                }
                else shield.setNear(false);
            }
            else{
                if(!mazeDestroyed)
                    Maze::destroy(maze, 15);
                mazeDestroyed = true;
            }
            if (((CamPos.x < door.getPosition().x + deltaPos * 2 && CamPos.x > door.getPosition().x - deltaPos * 2) &&
                (CamPos.z < door.getPosition().z + deltaPos * 2 && CamPos.z > door.getPosition().z - deltaPos * 2)))
            {
                door.setNear(true);
            }
            else door.setNear(false);

            if (((CamPos.x < keys[0].getPosition().x + deltaPos && CamPos.x > keys[0].getPosition().x - deltaPos) &&
                (CamPos.z < keys[0].getPosition().z + deltaPos && CamPos.z > keys[0].getPosition().z - deltaPos)))
            {
                keys[0].setNear(true);
                idxKeyNearest = 0;
            }
            else if (((CamPos.x < keys[1].getPosition().x + deltaPos && CamPos.x > keys[1].getPosition().x - deltaPos) &&
                (CamPos.z < keys[1].getPosition().z + deltaPos && CamPos.z > keys[1].getPosition().z - deltaPos)))
            {
                keys[1].setNear(true);
                idxKeyNearest = 1;
            }
            else
            {
                keys[0].setNear(false);
                keys[1].setNear(false);
            }
        }
        switch (gameState) {
            case 0: //initial state
                if (handleFire) {
                    gameState = 1;
                }
                break;
            case 1://0 or 1 key
            case 2:
                    if ((keys[0].isNear() || keys[1].isNear()) && handleFire) {
                        if (idxKeyNearest == 0 && !keys[0].isTaken())
                        {
                            keys[0].hide();
                            gameState++;
                        }
                        else if (idxKeyNearest == 1 && !keys[1].isTaken())
                        {
                            keys[1].hide();
                            gameState++;
                        }
                    }
                break;
            case 3://2 keys
                if (door.isNear() && handleFire && !used)
                {
                    door.setNear(false);
                    used = true;
                    keys[0].setScale(glm::vec3(1.0f));
                    //use of keys[0] as the key to open the door
                    keys[0].setShow(true);
                    keys[0].setPosition({ door.getPosition().x+deltaPos,0,door.getPosition().z - deltaPos });
                    keySpawnPos = keys[0].getPosition();
        
                    MoveCam = false;
                }
                if (!MoveCam && used) {
                    keys[0].setPosition({keys[0].getPosition().x - KEY_SPEED * m.x * deltaT,
                        keys[0].getPosition().y + KEY_SPEED * m.y * deltaT,
                        keys[0].getPosition().z + KEY_SPEED * m.z * deltaT});
                    
                    //quaternions to move the Key1 to open the door
                    
                    KeyRot = glm::quat(glm::vec3(0, -ROT_SPEED * deltaT * r.y, 0)) *
                    glm::quat(glm::vec3(-ROT_SPEED * deltaT * r.x, 0, 0)) *
                    glm::quat(glm::vec3(0, 0, ROT_SPEED * deltaT * r.z)) *
                    KeyRot;
                    if (correctKeyPosition()) {
                        open = true;
                        gameState = 4;
                        keys[0].setShow(false);
                    }
                }
                
           if (keys[0].isShowable()) {
            World = glm::mat4(1);
            World = glm::translate(World, glm::vec3(keys[0].getPosition())) *
            glm::mat4(KeyRot) * glm::scale(World, glm::vec3(keys[0].getScale()));
           }
           else {
            World = glm::mat4(0);
           }
            ubo.mMat = World * baseTr;
            ubo.mvpMat = ViewPrj * ubo.mMat;
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            DSKeys[0].map(currentImage, &ubo, sizeof(ubo), 0);
                
            if (used)
            {
                //press P to see the current position of the key
                if (glfwGetKey(window, GLFW_KEY_P)) {
                    if (!debounce) {
                        printVec3("KeyPos", keys[0].getPosition());
                        std::cout
                            << "Angle x:" << KeyRot.x
                            << "Angle y:" << KeyRot.y
                            << "Angle z:" << KeyRot.z
                            << "Angle w :" << KeyRot.w
                            <<"\n";
                        debounce = true;
                        curDebounce = GLFW_KEY_P;
                    }
                }
                else {
                    if ((curDebounce == GLFW_KEY_P) && debounce) {
                        debounce = false;
                        curDebounce = 0;
                    }
                }

                if (fire) {
                    if (!debounce) {
                        debounce = true;
                        curDebounce = GLFW_KEY_SPACE;
                        MoveCam = !MoveCam;
                        std::cout << "Switch!  " << (MoveCam ? "Camera" : "Key") << "\n";
                    }
                }
                else {
                    if ((curDebounce == GLFW_KEY_SPACE) && debounce) {
                        debounce = false;
                        curDebounce = 0;
                    }
                }
            }
            break;
        case 4://open door
                if(open){
                    if(!doorIsOpen){
                        if(door_animation_angle <= 90.0f){
                            door_animation_angle += deltaT*10;
                            door.setRotationAngle(-door_animation_angle);
                        }
                        else{
                            MoveCam = true;
                        }
                        ubo.mMat = baseTr;
                        ubo.mvpMat = ViewPrj * door.calculateWorldMatrix();
                        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
                        DSDoor.map(currentImage, &ubo, sizeof(ubo), 0);
                        keys[0].hide();
                        doorIsOpen = true;
                    }
                }
            break;
        default:
            break;
        }

        oub.visible = (gameState == 0) ? 1.0f : 0.0f;
        DSStart.map(currentImage, &oub, sizeof(oub), 0);

        oub.visible = ((keys[0].isNear() && ((gameState == 1 || gameState == 2) && !keys[0].isTaken())) || (keys[1].isNear() && ((gameState == 1 || gameState == 2) && !keys[1].isTaken()))) ? 1.0f : 0.0f;
        DSTake.map(currentImage, &oub, sizeof(oub), 0);

        oub.visible = (door.isNear() && gameState == 3 && !used) ? 1.0f : 0.0f;
        DSUse.map(currentImage, &oub, sizeof(oub), 0);
        
        oub.visible = (heart.isNear() && !heart.isTaken()) ? 1.0f : 0.0f;
        DSPick.map(currentImage, &oub, sizeof(oub), 0);
        
        
        oub.visible = (shield.isNear() && shield.isShowable()) ? 1.0f : 0.0f;
        DSGet.map(currentImage, &oub, sizeof(oub), 0);
    
        if(heart.isNear() && !heart.isTaken() && handleFire)
        {
            lives++;
            heart.hide();
        }
        heart.setRotationAngle(angle_animation);
        ubo.mMat = baseTr;
        ubo.mvpMat = ViewPrj * heart.calculateWorldMatrix();
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        DSHeart.map(currentImage,&ubo,sizeof(ubo),0);
        
        for(int i = 0;i<3;i++){
            oub.visible = i < lives ? 1.0f : 0.0f;
            DSSymbolHeart[i].map(currentImage,&oub,sizeof(oub),0);
        }
        
        oub.visible = (keys[0].isTaken() || keys[1].isTaken()) ? 1.0f : 0.0f;
        DSSymbolKey[0].map(currentImage,&oub,sizeof(oub),0);
        oub.visible = (keys[0].isTaken() && keys[1].isTaken()) ? 1.0f : 0.0f;
        DSSymbolKey[1].map(currentImage,&oub,sizeof(oub),0);


    
        if(shield.isNear() && !shield.isTaken() && handleFire)
        {
            shield.hide();
        }
        
        oub.visible = shield.isTaken() ? 1.0f : 0.0f ;
        DSProtection.map(currentImage,&oub,sizeof(oub),0);

        shield.setRotationAngle(angle_animation);
        ubo.mMat = baseTr;
        ubo.mvpMat = ViewPrj * shield.calculateWorldMatrix();
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        DSShield.map(currentImage,&ubo,sizeof(ubo),0);
        
       
        if(gameState!=3){
            keys[0].setRotationAngle(angle_animation);
            ubo.mMat = baseTr;
            ubo.mvpMat = ViewPrj * keys[0].calculateWorldMatrix();
            ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
            keys[0].setRotationAngle(0.0f);
            DSKeys[0].map(currentImage, &ubo, sizeof(ubo), 0);

        }
      

        keys[1].setRotationAngle(angle_animation);
        ubo.mMat = baseTr;
        ubo.mvpMat = ViewPrj * keys[1].calculateWorldMatrix();
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        DSKeys[1].map(currentImage, &ubo, sizeof(ubo), 0);
        

        uboMaze.mMat = baseTr;
        uboMaze.mvpMat = ViewPrj * uboMaze.mMat;
        uboMaze.nMat = glm::inverse(glm::transpose(uboMaze.mMat));
        DSMaze.map(currentImage, &uboMaze, sizeof(uboMaze), 0);
        DSMaze.map(currentImage, &guboMaze, sizeof(guboMaze), 1);

        
        uboRoom.mMat = baseTr;
        uboRoom.mvpMat = ViewPrj * uboRoom.mMat;
        uboRoom.nMat = glm::inverse(glm::transpose(uboRoom.mMat));
        DSRoom.map(currentImage, &uboRoom, sizeof(uboRoom), 0);
        DSRoom.map(currentImage, &guboRoom, sizeof(guboRoom), 1);
        
        uboMaze.mMat = baseTr;
        uboMaze.mvpMat = ViewPrj * uboMaze.mMat;
        uboMaze.nMat = glm::inverse(glm::transpose(uboMaze.mMat));
        DSFloorCeil.map(currentImage, &uboMaze, sizeof(uboMaze), 0);
        DSFloorCeil.map(currentImage, &guboMaze, sizeof(guboMaze), 1);
        
        oub.visible = CamPos.z > 18 ? 1.0f : 0.0f;
        win = oub.visible == 1.0f ? true : false;
        DSWin.map(currentImage,&oub,sizeof(oub),0);
        
        oub.visible = lives < 0 ? 1.0f : 0.0f;
        DSLose.map(currentImage,&oub,sizeof(oub),0);

        

        guboDecoration.lightDir = glm::vec3(cos(glm::radians(135.0f)) * cos(glm::radians(30.0f)), sin(glm::radians(135.0f)), cos(glm::radians(135.0f)) * sin(glm::radians(30.0f)));
        guboDecoration.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        guboDecoration.eyePos = CamPos;
     
        uboDecoration.mMat = baseTr;
        for(int i = 0;i<N;i++)
        {
            uboDecoration.mvpMat[i] = ViewPrj * crystal[i].calculateWorldMatrix();

        }
        
        uboDecoration.nMat = glm::inverse(glm::transpose(uboDecoration.mMat));
        DSCrystal.map(currentImage, &uboDecoration, sizeof(uboDecoration), 0);
        DSCrystal.map(currentImage, &guboDecoration, sizeof(guboDecoration), 1);
        
        uboDecoration.mMat = baseTr;
        for(int i = 0;i<N;i++)
        {
            uboDecoration.mvpMat[i] = ViewPrj * skull[i].calculateWorldMatrix();
        }
        uboDecoration.nMat = glm::inverse(glm::transpose(uboDecoration.mMat));
        DSSkull.map(currentImage, &uboDecoration, sizeof(uboDecoration), 0);
        DSSkull.map(currentImage, &guboDecoration, sizeof(guboDecoration), 1);
        
  
    }
   
    char** genMaze(int row, int col) {
        char** out = (char**)calloc(row, sizeof(char*));
        for (int i = 0; i < row; i++) {
            out[i] = (char*)malloc(col + 1);
            for (int j = 0; j < col; j++) {
                out[i][j] = '#';
            }
            out[i][col] = 0;
        }
        
        srand(time(NULL));
        
        int sr = (rand() % (row / 2)) * 2 + 1, sc = (rand() % (col / 2)) * 2 + 1;
        out[sr][sc] = ' ';
        
        Maze::recMaze(row, col, out, sr, sc);
        sc = (rand() % (col / 2)) * 2 + 1;
        setCamPos(sc);
        out[0][sc] = ' ';
        sc = (rand() % (col / 2)) * 2 + 1;
        exit = sc;
        setDoorEnemyPositions(exit);
        out[row - 1][sc] = ' ';
        keys[0].startPosition(out, 'K', row, col, 1.0f,0.5f,0.3f);
        keys[1].startPosition(out, 'K', row, col, 1.0f,0.5f,0.3f);
        heart.startPosition(out, 'H', row, col, 0.01f,0.5f,0.2f);
        shield.startPosition(out, 'S', row, col, 0.01f,0.5f,0.1f);
        for(int i =0;i<N;i++)
        {
            crystal[i].startPosition(out, 'C', row, col, 0.6f,0.1f,0.1f);
            skull[i].startPosition(out, 'B', row, col, 0.6f, 0.1f,0.01f);
        }
        return out;
    }
    
   
    void setCamPos(int sc) {
        CamPos = { sc+0.2,0.5,0 };
        camPosValidPosition = { sc+0.2,0.5,0 };
        xSaved = sc+0.2;
        enter = sc +0.2;
        zSaved = 0;
        CamAlpha = 180.0f;
        CamBeta = 0.0f;
    }
    void setDoorEnemyPositions(int sc) {
        door.setPosition ({sc,0.0,15});
        door.setScale({ 0.985,1,1 });
        door.setNear(false);
        door.setRotationAngle(0.0f);
        door.setShow(true);
        posEnemy = { sc+0.5,0.2,14 };
        posEnemyValid = { sc + 0.5,0.1,14 };
        xSavedE = sc + 0.5;
        zSavedE = 14;
    }
    bool correctKeyPosition() {
        bool insertedX = false;
        bool insertedY = false;
        bool quaternions = false;
        float e = 0.2755;
        float x= keys[0].getPosition().x,
              y= keys[0].getPosition().y,
              z= keys[0].getPosition().z;
        if (keys[0].getPosition().x - door.getPosition().x   >= 0.43 &&  keys[0].getPosition().x - door.getPosition().x <= 0.52) {
            insertedX = true;
        }
        else {
            x = keys[0].getPosition().x + e > (exit + 1) ? (exit + 1 - e) : keys[0].getPosition().x - e < exit ? exit + e : keys[0].getPosition().x;
            keys[0].setPosition({ x,y,z});
        }
        if (keys[0].getPosition().y >= 0.53 && keys[0].getPosition().y <= 0.59) {
            insertedY = true;
        }
        else {
            y = keys[0].getPosition().y - e < 0 ? 0 + e : (keys[0].getPosition().y + e > 1 ? 1 - e : keys[0].getPosition().y);
            keys[0].setPosition({ x,y,z});
        }
        if (abs(KeyRot.x) >= 0.39 && abs(KeyRot.x) <= 0.54 && abs(KeyRot.y) >= 0.41 && abs(KeyRot.y) <= 0.56 && abs(KeyRot.z) >= 0.43 && abs(KeyRot.z) <= 0.56 && abs(KeyRot.w) >= 0.44 && abs(KeyRot.w) <= 0.59) {
            quaternions = true;
        }
        else {
            quaternions = false;
        }

        if (insertedX && insertedY && quaternions) {
            if (keys[0].getPosition().z >= 14.6875) {
                return  true;
            }
        }
        else {
            z = keys[0].getPosition().z >= 14.6 ?  z = 14.6 : (keys[0].getPosition().z < keySpawnPos.z ? keySpawnPos.z : z);
            keys[0].setPosition({ x,y,z});
            return false;
        }
        return  false;
    }
    void correctPosition(glm::vec3& Position, glm::vec3& oldValidPosition, float* savedX, float* savedZ, float min, float max, bool e) {
                int a = floor(Position.z);
                int b = floor(Position.x);
                float ex = Position.x - b;
                float zed = Position.z - a;


                if ((a >= 0 && a<15 && b >= 0 && b<15) || (gameState == 3 && a == 14 && b == exit)) {

                    if (ex >max && b + 1 <15 && maze[a][b + 1] == '#') {
                        Position.x = b + max;
                    }
                    if (ex < min && b - 1 >= 0 && maze[a][b - 1] == '#') {
                         Position.x = b + min;
                    }

                    if (zed > max && a + 1 < 15 && maze[a + 1][b] == '#') {
                         Position.z = a + max;
                    }
                    if (zed < min && a - 1 >= 0 && maze[a - 1][b] == '#') {
                         Position.z = a + min;
                    }
                    
                    if ((Position.x == b + ex) && (Position.z == a + zed) && !e) {
                        if (ex <= min && zed <= min && a - 1 >= 0 && b - 1 >= 0 && maze[a - 1][b - 1] == '#') {
                            Position.x = b + min;
                            Position.z = a + min;
                        }
                        if (ex <= min && zed >= max && a - 1 >= 0 && b + 1 < 15 && maze[a - 1][b + 1] == '#') {
                            Position.x = b + min;
                            Position.z = a + max;
                        }
                        if (ex >= max && zed <= min && a + 1 < 15 && b - 1 >= 0 && maze[a + 1][b - 1] == '#') {
                            Position.x = b + max;
                            Position.z = a + min;
                        }
                        if (ex >= max && zed >= max && a + 1 >= 0 && b + 1 >= 0 && maze[a + 1][b + 1] == '#') {
                            Position.x = b + max;
                            Position.z = a + max;
                        }
                    }
                }

                if (a >= 0 && b >= 0 && a < 15 && b < 15 && maze[a][b] == '#') {
                    Position.x = *savedX;
                    Position.z = *savedZ;
                    oldValidPosition = Position;
                }
                else {
                    if (a < 0 || b <0 || a>=14 || b>=14) {
                        if (!(gameState >= 3 && a >= 14 && b == exit && !e)) {
                            Position = oldValidPosition;
                        }
                        else {
                            if (gameState == 3) {
                                if (Position.z >= 14.6) {
                                    Position.z = 14.6f;
                                }
                            }
                            else {
                                //inside the room
                                if (ex > 0.8)
                                    Position.x = b + 0.8;
                                if(ex < 0.35)
                                    Position.x = b + 0.35;
                                if (Position.z > 19.8) {
                                    Position.z = 19.8;
                                }
                            }
                            oldValidPosition = Position;
                        }
                    }
                    else {
                        oldValidPosition = Position;
                    }
                }
                *savedX = Position.x;
                *savedZ = Position.z;
                Position.y < 0.2f ? Position.y = 0.2f : Position.y > 0.8f ? Position.y = 0.8f : Position.y = Position.y;
    }
    void moveEnemy(float deltaT) {
        srand(time(NULL));
        const float SPEED_ENEMY = 1.0f;
        int a = rand() % 2;
        float b = a == 0 ? b = -1.0f : 1.0f;
        a = rand() % 4;
        float c = 90.0f * (a+1);


        glm::vec3 ux = glm::rotate(glm::mat4(1.0f), glm::radians(c), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
        glm::vec3 uz = glm::rotate(glm::mat4(1.0f), glm::radians(c), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);
        a = rand() % 2;
        
        if (a == 0) {
            posEnemy = posEnemy + SPEED_ENEMY * b * ux * deltaT;

        }
        else {
            posEnemy = posEnemy + SPEED_ENEMY * b * uz * deltaT;
        }
        correctPosition(posEnemy, posEnemyValid, &xSavedE, &zSavedE,0.4f,0.6f,true);
    }
};

// This is the main: probably you do not need to touch this!
int main() {
    MeshLoader app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
