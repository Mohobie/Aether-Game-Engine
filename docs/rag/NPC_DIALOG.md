# NPC Dialog System

## Quick Reference

```cpp
vge::NPCDialogSystem dialogSystem;

// Create dialog nodes
vge::DialogNode greeting;
greeting.id = "greeting";
greeting.speaker = "Villager";
greeting.text = "Hello, traveler! What brings you here?";

vge::DialogOption option1("I'm looking for quests.", "quests");
vge::DialogOption option2("Just passing through.", "");
vge::DialogOption option3("Tell me about this village.", "village_info");

greeting.options.push_back(option1);
greeting.options.push_back(option2);
greeting.options.push_back(option3);

dialogSystem.RegisterDialog(greeting);

// Start dialog
dialogSystem.StartDialog("greeting");

// Select option
dialogSystem.SelectOption(0); // Select first option
```

## Features

### Dialog Nodes
- **ID:** Unique identifier
- **Speaker:** Who is talking
- **Text:** Dialog content
- **Options:** Player response choices
- **Callbacks:** On enter, on select

### Dialog Options
- **Text:** What the player says
- **Next Node:** Where to go next
- **Callback:** Action when selected
- **Enabled:** Can be disabled

### System Features
- **Branching:** Multiple conversation paths
- **Callbacks:** Execute code on dialog events
- **State tracking:** Know if in dialog

## Implementation

```cpp
// Create dialog system
vge::NPCDialogSystem dialogs;

// Set callbacks
dialogs.onDialogStart = [](const vge::DialogNode& node) {
    ui.ShowDialogBox(node.speaker, node.text);
};

dialogs.onDialogEnd = []() {
    ui.HideDialogBox();
};

// Register dialogs
vge::DialogNode questDialog;
questDialog.id = "quests";
questDialog.speaker = "Villager";
questDialog.text = "I need help gathering wood. Can you help?";

vge::DialogOption accept("Yes, I'll help.", "");
accept.onSelect = []() {
    questSystem.StartQuest("gather_wood");
};

vge::DialogOption decline("Sorry, I'm busy.", "");

questDialog.options.push_back(accept);
questDialog.options.push_back(decline);

dialogs.RegisterDialog(questDialog);

// Interact with NPC
void OnNPCInteract() {
    dialogs.StartDialog("greeting");
}
```

## Files
- `src/game/npc_dialog.h`
- `src/game/npc_dialog.cpp`
