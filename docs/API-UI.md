# UI Module API

**Files:** `ui/ui_manager.h`, `ui/ui_element.h`, `ui/ui_button.h`, `ui/ui_label.h`, `ui/ui_panel.h`, `ui/ui_textbox.h`, `ui/ui_slider.h`, `ui/ui_checkbox.h`, `ui/ui_dropdown.h`, `ui/ui_image.h`, `ui/ui_progressbar.h`, `ui/ui_scrollbar.h`, `ui/ui_list.h`, `ui/ui_canvas.h`, `ui/ui_tooltip.h`

---

## `ui/ui_manager.h`

```cpp
namespace vge {
```

### `class UIManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `UIManager&` | `void` | Singleton |
| `Initialize` | `void` | `void` | Initialize UI |
| `Shutdown` | `void` | `void` | Shutdown UI |
| `Update` | `void` | `float deltaTime` | Update UI |
| `Render` | `void` | `void` | Render UI |
| `SetScreenSize` | `void` | `float width, float height` | Set screen dimensions |
| `GetScreenWidth` | `float` | `void` | Get screen width |
| `GetScreenHeight` | `float` | `void` | Get screen height |
| `SetScale` | `void` | `float scale` | Set UI scale |
| `GetScale` | `float` | `void` | Get UI scale |
| `SetTheme` | `void` | `const UITheme& theme` | Set UI theme |
| `GetTheme` | `UITheme` | `void` | Get current theme |
| `LoadTheme` | `bool` | `const std::string& path` | Load theme from file |
| `SaveTheme` | `bool` | `const std::string& path` | Save theme to file |
| `AddElement` | `void` | `UIElement* element` | Add element |
| `RemoveElement` | `void` | `UIElement* element` | Remove element |
| `GetElement` | `UIElement*` | `const std::string& name` | Get element by name |
| `GetRoot` | `UIElement*` | `void` | Get root canvas |
| `SetFocus` | `void` | `UIElement* element` | Set focused element |
| `GetFocus` | `UIElement*` | `void` | Get focused element |
| `ClearFocus` | `void` | `void` | Clear focus |
| `IsMouseOverUI` | `bool` | `void` | Check mouse over UI |
| `GetElementAt` | `UIElement*` | `float x, float y` | Get element at position |
| `ShowCursor` | `void` | `bool show` | Show/hide cursor |
| `IsCursorVisible` | `bool` | `void` | Check cursor visible |
| `SetCursorTexture` | `void` | `Texture* texture` | Set cursor texture |
| `SetCursorSize` | `void` | `float width, float height` | Set cursor size |
| `SetCursorHotspot` | `void` | `float x, float y` | Set cursor hotspot |
| `EnableInput` | `void` | `bool enable` | Enable/disable input |
| `IsInputEnabled` | `bool` | `void` | Check input enabled |
| `CreateCanvas` | `UICanvas*` | `const std::string& name` | Create canvas |
| `DestroyCanvas` | `void` | `const std::string& name` | Destroy canvas |
| `GetCanvas` | `UICanvas*` | `const std::string& name` | Get canvas |
| `GetAllCanvases` | `std::vector<UICanvas*>` | `void` | All canvases |
| `BringToFront` | `void` | `UIElement* element` | Bring to front |
| `SendToBack` | `void` | `UIElement* element` | Send to back |
| `SetDebugDraw` | `void` | `bool enable` | Toggle debug outlines |
| `IsDebugDrawEnabled` | `bool` | `void` | Check debug draw |

---

## `ui/ui_element.h`

### `enum class Anchor`
| Value | Description |
|-------|-------------|
| `TopLeft` | Anchor top-left |
| `TopCenter` | Anchor top-center |
| `TopRight` | Anchor top-right |
| `MiddleLeft` | Anchor middle-left |
| `MiddleCenter` | Anchor center |
| `MiddleRight` | Anchor middle-right |
| `BottomLeft` | Anchor bottom-left |
| `BottomCenter` | Anchor bottom-center |
| `BottomRight` | Anchor bottom-right |
| `Stretch` | Stretch to fill parent |

### `enum class Pivot`
| Value | Description |
|-------|-------------|
| `TopLeft` | Pivot top-left |
| `TopCenter` | Pivot top-center |
| `TopRight` | Pivot top-right |
| `CenterLeft` | Pivot center-left |
| `Center` | Pivot center |
| `CenterRight` | Pivot center-right |
| `BottomLeft` | Pivot bottom-left |
| `BottomCenter` | Pivot bottom-center |
| `BottomRight` | Pivot bottom-right |

### `struct RectTransform`
| Member | Type | Description |
|--------|------|-------------|
| `anchorMin` | `Vec2` | Min anchor point |
| `anchorMax` | `Vec2` | Max anchor point |
| `pivot` | `Vec2` | Pivot point (0-1) |
| `anchoredPosition` | `Vec2` | Position relative to anchor |
| `sizeDelta` | `Vec2` | Size offset from anchors |
| `rotation` | `float` | Rotation in degrees |
| `scale` | `Vec2` | Scale factor |

### `class UIElement`
Base class for all UI elements.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `UIElement` | (ctor) | `void` | Constructor |
| `SetName` | `void` | `const std::string& name` | Set name |
| `GetName` | `std::string` | `void` | Get name |
| `SetActive` | `void` | `bool active` | Set active |
| `IsActive` | `bool` | `void` | Check active |
| `SetVisible` | `void` | `bool visible` | Set visible |
| `IsVisible` | `bool` | `void` | Check visible |
| `SetInteractable` | `void` | `bool interactable` | Set interactable |
| `IsInteractable` | `bool` | `void` | Check interactable |
| `SetParent` | `void` | `UIElement* parent` | Set parent |
| `GetParent` | `UIElement*` | `void` | Get parent |
| `GetChildren` | `std::vector<UIElement*>` | `void` | Get children |
| `AddChild` | `void` | `UIElement* child` | Add child |
| `RemoveChild` | `void` | `UIElement* child` | Remove child |
| `RemoveAllChildren` | `void` | `void` | Remove all children |
| `SetRectTransform` | `void` | `const RectTransform& rect` | Set rect transform |
| `GetRectTransform` | `RectTransform` | `void` | Get rect transform |
| `GetScreenPosition` | `Vec2` | `void` | Get screen position |
| `GetScreenSize` | `Vec2` | `void` | Get screen size |
| `GetScreenRect` | `std::tuple<Vec2, Vec2>` | `void` | Get screen rect (min, max) |
| `ContainsPoint` | `bool` | `float x, float y` | Check contains point |
| `SetPosition` | `void` | `float x, float y` | Set position |
| `GetPosition` | `Vec2` | `void` | Get position |
| `SetSize` | `void` | `float width, float height` | Set size |
| `GetSize` | `Vec2` | `void` | Get size |
| `SetRotation` | `void` | `float rotation` | Set rotation |
| `GetRotation` | `float` | `void` | Get rotation |
| `SetScale` | `void` | `float scaleX, float scaleY` | Set scale |
| `GetScale` | `Vec2` | `void` | Get scale |
| `SetAnchor` | `void` | `Anchor anchor` | Set anchor |
| `GetAnchor` | `Anchor` | `void` | Get anchor |
| `SetPivot` | `void` | `Pivot pivot` | Set pivot |
| `GetPivot` | `Pivot` | `void` | Get pivot |
| `SetColor` | `void` | `const Vec3& color` | Set color |
| `GetColor` | `Vec3` | `void` | Get color |
| `SetAlpha` | `void` | `float alpha` | Set alpha (0-1) |
| `GetAlpha` | `float` | `void` | Get alpha |
| `SetSortingOrder` | `void` | `int order` | Set sort order |
| `GetSortingOrder` | `int` | `void` | Get sort order |
| `SetRaycastTarget` | `void` | `bool target` | Set raycast target |
| `IsRaycastTarget` | `bool` | `void` | Check raycast target |
| `SetNavigation` | `void` | `UIElement* up, UIElement* down, UIElement* left, UIElement* right` | Set navigation |
| `GetNavigationUp` | `UIElement*` | `void` | Get up navigation |
| `GetNavigationDown` | `UIElement*` | `void` | Get down navigation |
| `GetNavigationLeft` | `UIElement*` | `void` | Get left navigation |
| `GetNavigationRight` | `UIElement*` | `void` | Get right navigation |
| `OnPointerEnter` | `void` | `void` | Pointer enter event |
| `OnPointerExit` | `void` | `void` | Pointer exit event |
| `OnPointerDown` | `void` | `void` | Pointer down event |
| `OnPointerUp` | `void` | `void` | Pointer up event |
| `OnPointerClick` | `void` | `void` | Pointer click event |
| `OnPointerDrag` | `void` | `float deltaX, float deltaY` | Pointer drag event |
| `OnPointerScroll` | `void` | `float delta` | Pointer scroll event |
| `OnSelect` | `void` | `void` | Select event |
| `OnDeselect` | `void` | `void` | Deselect event |
| `OnUpdate` | `void` | `float deltaTime` | Update event |
| `OnRender` | `void` | `void` | Render event |
| `SetOnClick` | `void` | `std::function<void()> callback` | Set click callback |
| `SetOnPointerEnter` | `void` | `std::function<void()> callback` | Set enter callback |
| `SetOnPointerExit` | `void` | `std::function<void()> callback` | Set exit callback |
| `SetOnPointerDown` | `void` | `std::function<void()> callback` | Set down callback |
| `SetOnPointerUp` | `void` | `std::function<void()> callback` | Set up callback |
| `SetOnValueChanged` | `void` | `std::function<void()> callback` | Set value changed callback |
| `SetTooltip` | `void` | `const std::string& text` | Set tooltip text |
| `GetTooltip` | `std::string` | `void` | Get tooltip text |
| `SetTooltipDelay` | `void` | `float delay` | Set tooltip delay |
| `GetTooltipDelay` | `float` | `void` | Get tooltip delay |
| `SetTooltipOffset` | `void` | `float x, float y` | Set tooltip offset |
| `GetTooltipOffset` | `Vec2` | `void` | Get tooltip offset |
| `SetAnimation` | `void` | `UIAnimation* animation` | Set animation |
| `GetAnimation` | `UIAnimation*` | `void` | Get animation |
| `PlayAnimation` | `void` | `const std::string& name` | Play animation |
| `StopAnimation` | `void` | `void` | Stop animation |
| `IsAnimating` | `bool` | `void` | Check animating |
| `SetLayoutGroup` | `void` | `UILayoutGroup* layout` | Set layout group |
| `GetLayoutGroup` | `UILayoutGroup*` | `void` | Get layout group |
| `ForceLayout` | `void` | `void` | Force layout update |
| `SetData` | `void` | `const std::string& key, const std::string& value` | Set user data |
| `GetData` | `std::string` | `const std::string& key` | Get user data |
| `HasData` | `bool` | `const std::string& key` | Check has data |
| `Serialize` | `std::string` | `void` | Serialize |
| `Deserialize` | `void` | `const std::string& data` | Deserialize |
