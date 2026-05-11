# Common Game Patterns

This document contains common patterns and solutions for typical game development scenarios.

## Player Controller

```cpp
class PlayerController {
    vge::EntityID player;
    vge::Camera* camera;
    vge::World* world;
    vge::InputManager* input;
    
    float moveSpeed = 5.0f;
    float sprintSpeed = 10.0f;
    float jumpForce = 10.0f;
    float mouseSensitivity = 0.1f;
    
    bool isGrounded = false;
    bool isSprinting = false;
    bool isFlying = false;
    
public:
    void Update(float dt) {
        auto* transform = entityManager.GetComponent<TransformComponent>(player);
        auto* physics = entityManager.GetComponent<PhysicsComponent>(player);
        
        // Look
        float lookX = input->GetMouseDeltaX() * mouseSensitivity;
        float lookY = input->GetMouseDeltaY() * mouseSensitivity;
        camera->Rotate(lookX, -lookY);
        
        // Movement
        vge::Vec3 moveInput(0, 0, 0);
        if (input->GetKey(Key::W)) moveInput.z += 1;
        if (input->GetKey(Key::S)) moveInput.z -= 1;
        if (input->GetKey(Key::A)) moveInput.x -= 1;
        if (input->GetKey(Key::D)) moveInput.x += 1;
        
        if (moveInput.Length() > 0) {
            moveInput = moveInput.Normalized();
        }
        
        // Sprint
        isSprinting = input->GetKey(Key::LeftShift);
        float currentSpeed = isSprinting ? sprintSpeed : moveSpeed;
        
        // Calculate movement direction
        vge::Vec3 forward = camera->GetForward();
        vge::Vec3 right = camera->GetRight();
        forward.y = 0;
        forward = forward.Normalized();
        
        vge::Vec3 worldMove = forward * moveInput.z + right * moveInput.x;
        
        if (isFlying) {
            // Free movement in all directions
            worldMove += camera->GetUp() * moveInput.y;
            physics->SetVelocity(worldMove * currentSpeed);
        } else {
            // Ground movement
            physics->SetVelocity(vge::Vec3(worldMove.x * currentSpeed, physics->GetVelocity().y, worldMove.z * currentSpeed));
            
            // Jump
            if (input->GetKeyDown(Key::Space) && isGrounded) {
                physics->AddForce(vge::Vec3(0, jumpForce, 0));
            }
            
            // Check grounded
            isGrounded = CheckGrounded();
        }
        
        // Sync camera to player
        camera->SetPosition(transform->position + vge::Vec3(0, 1.6f, 0));
        
        // Block interaction
        if (input->GetMouseButtonDown(MouseButton::Left)) {
            BreakBlock();
        }
        if (input->GetMouseButtonDown(MouseButton::Right)) {
            PlaceBlock();
        }
    }
    
private:
    bool CheckGrounded() {
        vge::Ray ray(transform->position, vge::Vec3(0, -1, 0));
        vge::RaycastHit hit;
        return world->Raycast(ray, 1.1f, hit);
    }
    
    void BreakBlock() {
        vge::Ray ray(camera->GetPosition(), camera->GetForward());
        vge::RaycastHit hit;
        if (world->Raycast(ray, 5.0f, hit)) {
            world->SetBlock(hit.position, vge::BlockType::Air);
            particleSystem.SpawnBreakEffect(hit.position, hit.blockType);
            audio->Play("break");
        }
    }
    
    void PlaceBlock() {
        vge::Ray ray(camera->GetPosition(), camera->GetForward());
        vge::RaycastHit hit;
        if (world->Raycast(ray, 5.0f, hit)) {
            vge::Vec3 placePos = hit.position + hit.normal;
            world->SetBlock(placePos, inventory.GetSelectedBlock());
            audio->Play("place");
        }
    }
};
```

## Inventory System

```cpp
class Inventory {
    static const int HOTBAR_SIZE = 9;
    static const int INVENTORY_SIZE = 36;
    
    struct ItemStack {
        std::string itemId;
        int count = 0;
        int maxStack = 64;
        
        bool IsEmpty() const { return count <= 0; }
        bool IsFull() const { return count >= maxStack; }
        int Add(int amount) {
            int canAdd = std::min(amount, maxStack - count);
            count += canAdd;
            return amount - canAdd; // Return remainder
        }
        int Remove(int amount) {
            int canRemove = std::min(amount, count);
            count -= canRemove;
            return canRemove;
        }
    };
    
    std::array<ItemStack, INVENTORY_SIZE> slots;
    int selectedHotbarSlot = 0;
    
public:
    bool AddItem(const std::string& itemId, int count = 1) {
        // Try to stack with existing
        for (auto& slot : slots) {
            if (slot.itemId == itemId && !slot.IsFull()) {
                count = slot.Add(count);
                if (count <= 0) return true;
            }
        }
        
        // Find empty slot
        for (auto& slot : slots) {
            if (slot.IsEmpty()) {
                slot.itemId = itemId;
                count = slot.Add(count);
                if (count <= 0) return true;
            }
        }
        
        return false; // Inventory full
    }
    
    bool RemoveItem(const std::string& itemId, int count = 1) {
        for (auto& slot : slots) {
            if (slot.itemId == itemId) {
                int removed = slot.Remove(count);
                count -= removed;
                if (slot.IsEmpty()) slot.itemId = "";
                if (count <= 0) return true;
            }
        }
        return false;
    }
    
    bool HasItem(const std::string& itemId, int count = 1) {
        int total = 0;
        for (const auto& slot : slots) {
            if (slot.itemId == itemId) {
                total += slot.count;
                if (total >= count) return true;
            }
        }
        return false;
    }
    
    int GetItemCount(const std::string& itemId) {
        int total = 0;
        for (const auto& slot : slots) {
            if (slot.itemId == itemId) total += slot.count;
        }
        return total;
    }
    
    void SelectHotbarSlot(int slot) {
        selectedHotbarSlot = std::clamp(slot, 0, HOTBAR_SIZE - 1);
    }
    
    int GetSelectedSlot() const { return selectedHotbarSlot; }
    
    const ItemStack& GetHotbarItem(int slot) const {
        return slots[slot];
    }
    
    const ItemStack& GetSelectedItem() const {
        return slots[selectedHotbarSlot];
    }
    
    std::string GetSelectedBlock() const {
        const auto& item = GetSelectedItem();
        if (item.itemId == "stone") return vge::BlockType::Stone;
        if (item.itemId == "dirt") return vge::BlockType::Dirt;
        if (item.itemId == "wood") return vge::BlockType::Wood;
        return vge::BlockType::Air;
    }
};
```

## Crafting System

```cpp
struct Recipe {
    std::string result;
    int resultCount;
    std::map<std::string, int> ingredients;
};

class CraftingSystem {
    std::vector<Recipe> recipes;
    
public:
    void Initialize() {
        // Add recipes
        recipes.push_back({"planks", 4, {{"wood", 1}}});
        recipes.push_back({"stick", 4, {{"planks", 2}}});
        recipes.push_back({"pickaxe", 1, {{"stick", 2}, {"stone", 3}}});
        recipes.push_back({"sword", 1, {{"stick", 1}, {"stone", 2}}});
        recipes.push_back({"torch", 4, {{"stick", 1}, {"coal", 1}}});
    }
    
    bool CanCraft(const Recipe& recipe, const Inventory& inventory) {
        for (const auto& [item, count] : recipe.ingredients) {
            if (!inventory.HasItem(item, count)) {
                return false;
            }
        }
        return true;
    }
    
    bool Craft(const Recipe& recipe, Inventory& inventory) {
        if (!CanCraft(recipe, inventory)) return false;
        
        // Remove ingredients
        for (const auto& [item, count] : recipe.ingredients) {
            inventory.RemoveItem(item, count);
        }
        
        // Add result
        inventory.AddItem(recipe.result, recipe.resultCount);
        return true;
    }
    
    std::vector<const Recipe*> GetAvailableRecipes(const Inventory& inventory) {
        std::vector<const Recipe*> available;
        for (const auto& recipe : recipes) {
            if (CanCraft(recipe, inventory)) {
                available.push_back(&recipe);
            }
        }
        return available;
    }
};
```

## Save/Load System

```cpp
class SaveSystem {
public:
    void SaveWorld(const vge::World& world, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        
        // Write version
        int version = 1;
        file.write((char*)&version, sizeof(version));
        
        // Write chunks
        auto chunks = world.GetAllChunks();
        int chunkCount = chunks.size();
        file.write((char*)&chunkCount, sizeof(chunkCount));
        
        for (const auto* chunk : chunks) {
            // Write chunk coordinates
            int cx = chunk->GetX();
            int cz = chunk->GetZ();
            file.write((char*)&cx, sizeof(cx));
            file.write((char*)&cz, sizeof(cz));
            
            // Write block data (compressed)
            auto blockData = chunk->GetBlockData();
            file.write((char*)blockData.data(), blockData.size());
        }
        
        // Write entities
        auto entities = entityManager.GetAllEntities();
        int entityCount = entities.size();
        file.write((char*)&entityCount, sizeof(entityCount));
        
        for (auto entity : entities) {
            SerializeEntity(file, entity);
        }
        
        // Write player data
        SerializePlayer(file);
        
        file.close();
    }
    
    void LoadWorld(vge::World& world, const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return;
        
        // Read version
        int version;
        file.read((char*)&version, sizeof(version));
        
        // Read chunks
        int chunkCount;
        file.read((char*)&chunkCount, sizeof(chunkCount));
        
        for (int i = 0; i < chunkCount; ++i) {
            int cx, cz;
            file.read((char*)&cx, sizeof(cx));
            file.read((char*)&cz, sizeof(cz));
            
            // Read block data
            std::vector<uint8_t> blockData(16 * 16 * 16);
            file.read((char*)blockData.data(), blockData.size());
            
            // Create chunk
            world.LoadChunkData(cx, cz, blockData);
        }
        
        // Read entities
        int entityCount;
        file.read((char*)&entityCount, sizeof(entityCount));
        
        for (int i = 0; i < entityCount; ++i) {
            DeserializeEntity(file);
        }
        
        // Read player data
        DeserializePlayer(file);
        
        file.close();
    }
    
private:
    void SerializeEntity(std::ofstream& file, vge::EntityID entity) {
        // Write entity ID and components
        auto* transform = entityManager.GetComponent<TransformComponent>(entity);
        if (transform) {
            file.write((char*)&transform->position, sizeof(transform->position));
            file.write((char*)&transform->rotation, sizeof(transform->rotation));
        }
        
        // ... serialize other components
    }
    
    void DeserializeEntity(std::ifstream& file) {
        vge::EntityID entity = entityManager.CreateEntity();
        
        auto* transform = entityManager.AddComponent<TransformComponent>(entity);
        file.read((char*)&transform->position, sizeof(transform->position));
        file.read((char*)&transform->rotation, sizeof(transform->rotation));
        
        // ... deserialize other components
    }
};
```

## Game Loop

```cpp
class Game {
    vge::Window window;
    vge::Renderer renderer;
    vge::World world;
    vge::PlayerController player;
    vge::UISystem ui;
    vge::AudioEngine audio;
    
    bool running = true;
    float targetFPS = 60.0f;
    float fixedTimestep = 1.0f / 60.0f;
    float accumulator = 0.0f;
    
public:
    void Run() {
        Initialize();
        
        float lastTime = GetTime();
        
        while (running) {
            float currentTime = GetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;
            
            // Cap delta time to prevent spiral of death
            if (deltaTime > 0.1f) deltaTime = 0.1f;
            
            // Process input
            ProcessInput();
            
            // Fixed timestep physics
            accumulator += deltaTime;
            while (accumulator >= fixedTimestep) {
                FixedUpdate(fixedTimestep);
                accumulator -= fixedTimestep;
            }
            
            // Variable timestep update
            Update(deltaTime);
            
            // Render
            Render();
            
            // Frame limiting
            float frameTime = GetTime() - currentTime;
            float targetFrameTime = 1.0f / targetFPS;
            if (frameTime < targetFrameTime) {
                Sleep(targetFrameTime - frameTime);
            }
        }
        
        Shutdown();
    }
    
private:
    void Initialize() {
        window.Initialize(1920, 1080, "My Game");
        renderer.Initialize(1920, 1080);
        world.Initialize();
        player.Initialize(&world, &renderer.GetCamera());
        ui.Initialize(&renderer);
        audio.Initialize();
        
        // Load resources
        LoadResources();
    }
    
    void ProcessInput() {
        window.PollEvents();
        
        if (window.ShouldClose() || input.GetKeyDown(Key::Escape)) {
            running = false;
        }
    }
    
    void FixedUpdate(float dt) {
        // Physics
        physicsWorld.Update(dt);
        
        // AI
        aiSystem.Update(dt);
    }
    
    void Update(float dt) {
        // Player
        player.Update(dt);
        
        // World
        world.Update(dt);
        
        // Entities
        entityManager.Update(dt);
        
        // UI
        ui.Update(dt);
        
        // Audio
        audio.Update(dt);
        
        // Particles
        particleSystem.Update(dt);
    }
    
    void Render() {
        renderer.Clear(0.2f, 0.3f, 0.4f);
        
        // World
        renderer.RenderWorld(world, player.GetCamera());
        
        // Entities
        renderer.RenderEntities(entityManager);
        
        // Particles
        renderer.RenderParticles(particleSystem);
        
        // UI
        renderer.RenderUI(ui);
        
        renderer.Present();
    }
    
    void Shutdown() {
        // Save game
        saveSystem.SaveWorld(world, "autosave.dat");
        
        // Cleanup
        audio.Shutdown();
        ui.Shutdown();
        world.Shutdown();
        renderer.Shutdown();
        window.Shutdown();
    }
};
```

## Object Pooling

```cpp
template<typename T>
class ObjectPool {
    std::vector<std::unique_ptr<T>> available;
    std::vector<T*> inUse;
    std::function<std::unique_ptr<T>()> factory;
    size_t maxSize;
    
public:
    ObjectPool(size_t initialSize, size_t maxSize, std::function<std::unique_ptr<T>()> factory)
        : maxSize(maxSize), factory(factory) {
        for (size_t i = 0; i < initialSize; ++i) {
            available.push_back(factory());
        }
    }
    
    T* Acquire() {
        if (available.empty()) {
            if (inUse.size() >= maxSize) {
                return nullptr; // Pool exhausted
            }
            available.push_back(factory());
        }
        
        T* obj = available.back().get();
        available.pop_back();
        inUse.push_back(obj);
        return obj;
    }
    
    void Release(T* obj) {
        auto it = std::find(inUse.begin(), inUse.end(), obj);
        if (it != inUse.end()) {
            inUse.erase(it);
            // Find the unique_ptr and move it back to available
            // (implementation depends on how you track ownership)
        }
    }
    
    void Clear() {
        inUse.clear();
        // Reset all to available
    }
};

// Usage
ObjectPool<Projectile> projectilePool(100, 1000, []() {
    return std::make_unique<Projectile>();
});

void FireWeapon() {
    Projectile* proj = projectilePool.Acquire();
    if (proj) {
        proj->Initialize(muzzlePosition, fireDirection);
    }
}

void UpdateProjectiles(float dt) {
    for (auto* proj : activeProjectiles) {
        proj->Update(dt);
        if (proj->ShouldDestroy()) {
            projectilePool.Release(proj);
        }
    }
}
```

## Event System

```cpp
class EventSystem {
    using EventHandler = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<EventHandler>> handlers;
    
public:
    template<typename T>
    void Subscribe(std::function<void(const T&)> handler) {
        handlers[typeid(T)].push_back([handler](const void* event) {
            handler(*static_cast<const T*>(event));
        });
    }
    
    template<typename T>
    void Emit(const T& event) {
        auto it = handlers.find(typeid(T));
        if (it != handlers.end()) {
            for (auto& handler : it->second) {
                handler(&event);
            }
        }
    }
};

// Usage
struct DamageEvent {
    vge::EntityID attacker;
    vge::EntityID victim;
    float damage;
};

struct DeathEvent {
    vge::EntityID entity;
};

// Subscribe
eventSystem.Subscribe<DamageEvent>([](const DamageEvent& e) {
    auto* health = entityManager.GetComponent<HealthComponent>(e.victim);
    if (health) {
        health->TakeDamage(e.damage);
        if (health->IsDead()) {
            eventSystem.Emit(DeathEvent{e.victim});
        }
    }
});

eventSystem.Subscribe<DeathEvent>([](const DeathEvent& e) {
    // Spawn death particles
    auto* transform = entityManager.GetComponent<TransformComponent>(e.entity);
    if (transform) {
        particleSystem.SpawnExplosion(transform->position);
    }
    
    // Drop loot
    lootSystem.SpawnLoot(transform->position);
    
    // Remove entity
    entityManager.DestroyEntity(e.entity);
});

// Emit
eventSystem.Emit(DamageEvent{player, enemy, 25.0f});
```

## State Machine (Game States)

```cpp
class GameStateMachine {
    enum class State {
        Boot,
        MainMenu,
        Loading,
        Playing,
        Paused,
        GameOver
    };
    
    State currentState = State::Boot;
    State previousState = State::Boot;
    
public:
    void ChangeState(State newState) {
        ExitState(currentState);
        previousState = currentState;
        currentState = newState;
        EnterState(currentState);
    }
    
    void Update(float dt) {
        switch (currentState) {
            case State::MainMenu:
                UpdateMainMenu(dt);
                break;
            case State::Playing:
                UpdatePlaying(dt);
                break;
            case State::Paused:
                UpdatePaused(dt);
                break;
            // ...
        }
    }
    
    void Render() {
        switch (currentState) {
            case State::MainMenu:
                RenderMainMenu();
                break;
            case State::Playing:
                RenderPlaying();
                break;
            case State::Paused:
                RenderPlaying(); // Render game behind pause menu
                RenderPauseMenu();
                break;
            // ...
        }
    }
    
private:
    void EnterState(State state) {
        switch (state) {
            case State::MainMenu:
                audio.PlayMusic("menu");
                ui.ShowMainMenu();
                break;
            case State::Playing:
                audio.PlayMusic("gameplay");
                input.SetCursorLocked(true);
                break;
            case State::Paused:
                input.SetCursorLocked(false);
                ui.ShowPauseMenu();
                break;
            // ...
        }
    }
    
    void ExitState(State state) {
        switch (state) {
            case State::MainMenu:
                ui.HideMainMenu();
                break;
            case State::Playing:
                // Save game
                break;
            case State::Paused:
                ui.HidePauseMenu();
                break;
            // ...
        }
    }
    
    void UpdateMainMenu(float dt) {
        if (input.GetMouseButtonDown(MouseButton::Left)) {
            // Check button clicks
            if (ui.IsButtonHovered("play")) {
                ChangeState(State::Loading);
            }
            if (ui.IsButtonHovered("quit")) {
                QuitGame();
            }
        }
    }
    
    void UpdatePlaying(float dt) {
        if (input.GetKeyDown(Key::Escape)) {
            ChangeState(State::Paused);
        }
        
        game.Update(dt);
    }
    
    void UpdatePaused(float dt) {
        if (input.GetKeyDown(Key::Escape)) {
            ChangeState(State::Playing);
        }
    }
};
```

## Loading Screen

```cpp
class LoadingScreen {
    float progress = 0.0f;
    std::string status = "Loading...";
    std::vector<std::function<void()>> loadTasks;
    size_t currentTask = 0;
    
public:
    void AddTask(const std::string& name, std::function<void()> task) {
        loadTasks.push_back(task);
    }
    
    void Update(float dt) {
        if (currentTask < loadTasks.size()) {
            status = "Loading... " + std::to_string(currentTask + 1) + "/" + std::to_string(loadTasks.size());
            
            // Execute one task per frame to show progress
            loadTasks[currentTask]();
            currentTask++;
            
            progress = (float)currentTask / loadTasks.size();
        }
    }
    
    bool IsComplete() const {
        return currentTask >= loadTasks.size();
    }
    
    void Render(vge::UISystem* ui) {
        // Background
        ui->DrawRect(0, 0, 1920, 1080, vge::Vec4(0, 0, 0, 1));
        
        // Progress bar background
        ui->DrawRect(460, 500, 1000, 40, vge::Vec4(0.2f, 0.2f, 0.2f, 1));
        
        // Progress bar fill
        ui->DrawRect(460, 500, 1000 * progress, 40, vge::Vec4(0, 0.8f, 0, 1));
        
        // Status text
        ui->DrawText(960, 480, status, vge::Vec4(1, 1, 1, 1), 24, TextAlign::Center);
        
        // Percentage
        ui->DrawText(960, 560, std::to_string((int)(progress * 100)) + "%", vge::Vec4(1, 1, 1, 1), 20, TextAlign::Center);
    }
};

// Usage
LoadingScreen loading;
loading.AddTask("Textures", []() { textureManager.LoadAll(); });
loading.AddTask("Models", []() { modelManager.LoadAll(); });
loading.AddTask("Sounds", []() { audioManager.LoadAll(); });
loading.AddTask("World", []() { world.GenerateSpawnArea(); });

while (!loading.IsComplete()) {
    loading.Update(dt);
    loading.Render(ui);
    renderer.Present();
}
```
