/// For DDAutoPinConfig;
/// note that _h is for internal use only
/// @since v0.9.9-r50
struct DDAutoPinGroupHeader {
  String _h;
};

/// experimental:
/// @since v0.9.9-r50
extern DDAutoPinGroupHeader DDAutoPinStackedGroupHeader(int left, int top, int right, int bottom);

/// experimental:
/// For DDAutoPinConfig;
/// for auto pinning the contained in a grid form
/// @param columnCount number of columns
/// @param rowCount number of rows  
/// @param align if not empty, the default grid cell align (e.g. "LB") -- left align "L"; right align "R"; top align "T"; bottom align "B"
extern DDAutoPinGroupHeader DDAutoPinGridGroupHeader(int columnCount, int rowCount, const String& align = "", bool sameColumnWidth = false, bool sameRowHeight = false);

/// experimental:
/// For DDAutoPinConfig;
/// explicitly specify a grid cell, when columnSpan or rowSpan not 1;
/// note that a grid cell must contain a single "auto pin item"
/// @param columnSpan number of columns to span
/// @param rowSpan number of rows to span
/// @param align if not empty, the grid cell align (e.g. "LB") -- left align "L"; right align "R"; top align "T"; bottom align "B"
extern DDAutoPinGroupHeader DDAutoPinGridCellHeader(int columnSpan, int rowSpan, const String& align = "");

/// experimental:
/// The same purpose of DDAutoPinGridCellHeader
extern DDAutoPinGroupHeader DDAutoPinGridCellAlignHeader(const String& align, int columnSpan = 1, int rowSpan = 1);


/// @brief
/// Class for building "auto pin" config, to be passed to DumbDisplay::configAutoPin().
/// @since v0.9.7-r2
class DDAutoPinConfig {
  public:
    /// @param dir directory of layers at the top level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    /// @param nestedDepth maximum depth of nesting; default is 5
    DDAutoPinConfig(char dir, int nestedDepth = 5) {
      config = String(dir) + "(";
      depth = 0;
      started = new bool[nestedDepth + 1];
      started[depth] = false;
    }
    /// @param header group header
    /// @param nestedDepth maximum depth of nesting; default is 5
    DDAutoPinConfig(DDAutoPinGroupHeader header, int nestedDepth = 5) {
      config = header._h + "(";
      depth = 0;
      started = new bool[nestedDepth + 1];
      started[depth] = false;
    }
    ~DDAutoPinConfig() {
      delete started;
    }
  public:
    /// begin a layer group, creating a new level of nesting
    /// @param dir directory of layers at the new level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    DDAutoPinConfig& beginGroup(char dir) {
      addConfig(String(dir) + "(");
      depth += 1;
      // int maxDepth = sizeof(started) / sizeof(bool) - 1;
      // if (depth > maxDepth) {
      //   // not expected
      //   depth = maxDepth;
      // }
      started[depth] = false;
      return *this;
    }  
    /// begin a layer group, creating a new level of nesting
    /// @param dir directory of layers at the new level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    DDAutoPinConfig& beginGroup(DDAutoPinGroupHeader header) {
      addConfig(header._h + "(");
      depth += 1;
      started[depth] = false;
      return *this;
    }
    /// end a begun group, returning to the previous level of nesting
    DDAutoPinConfig& endGroup() {
      config.concat(')');
      depth -= 1;
      if (depth < 0) {
        // not expected
        depth = 0;
      }
      return *this;
    }
    /// begin a layer group, with specified padding
    /// @param dir directory of layers at the new level; can be 'H' for horizontal,  'V' for vertical and 'S' for stacked
    /// @param left left padding (% of the contained area width)
    /// @param top top padding  (% of the contained area height) 
    /// @param right right padding (% of the contained area width)
    /// @param bottom bottom padding (% of the contained area height)
    DDAutoPinConfig& beginPaddedGroup(char dir, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(");
      depth += 1;
      // int maxDepth = sizeof(started) / sizeof(bool) - 1;
      // if (depth > maxDepth) {
      //   // not expected
      //   depth = maxDepth;
      // }
      started[depth] = false;
      return beginGroup(dir);
    }  
    /// end begun padded group
    DDAutoPinConfig& endPaddedGroup() {
      endGroup();
      config.concat(')');
      depth -= 1;
      return *this;
    }
    /// add a layer to the current level
    DDAutoPinConfig& addLayer(DDLayer* layer) {
      if (layer != NULL) {
        addConfig(layer->getLayerId());
      }
      return *this;
    }
    /// add a layer to the current level
    DDAutoPinConfig& addLayer(DDLayer& layer) {
      addConfig(layer.getLayerId());
      return *this;
    }
    /// add a layer to the current level
    DDAutoPinConfig& addLayer(DDLayerHandle layerHandle) {
      addConfig(String(layerHandle._h));
      return *this;
    }
    /// add a layer, with specified padding
    /// @param left left padding (% of the contained area width)
    /// @param top top padding  (% of the contained area height) 
    /// @param right right padding (% of the contained area width)
    /// @param bottom bottom padding (% of the contained area height)
    DDAutoPinConfig& addPaddedLayer(DDLayer* layer, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(" + String(layer->getLayerId()) + ")");
      return *this;
    }  
    /// add a layer, with specified padding
    /// @param left left padding (% of the contained area width)
    /// @param top top padding  (% of the contained area height) 
    /// @param right right padding (% of the contained area width)
    /// @param bottom bottom padding (% of the contained area height)
    DDAutoPinConfig& addPaddedLayer(DDLayer& layer, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(" + String(layer.getLayerId()) + ")");
      return *this;
    }  
    /// add a layer, with specified padding
    /// @param left left padding (% of the contained area width)
    /// @param top top padding  (% of the contained area height) 
    /// @param right right padding (% of the contained area width)
    /// @param bottom bottom padding (% of the contained area height)
    DDAutoPinConfig& addPaddedLayer(DDLayerHandle layerHandle, int left, int top, int right, int bottom) {
      addConfig(String("S/") + String(left) + "-" + String(top) + "-" + String(right) + "-" + String(bottom) + "(" + String(layerHandle._h) + ")");
      return *this;
    }  
    /// add spacer, which is a placeholder layer with the specified size
    /// @param width width of the placeholder layer
    /// @param height height of the placeholder layer
    DDAutoPinConfig& addSpacer(int width, int height) {
      addConfig(String("<") + String(width) + "x" + String(height) + String(">"));
      return *this;
    }
    /// add the layout direction for the layers not included
    /// @param dir 'H' / 'V' / 'S
    DDAutoPinConfig& addRemainingGroup(char dir) {
      addConfig(String(dir) + "(*)");
      return *this;
    }
    /// build the "auto pin" config string, to be passed to DumbDisplay::configAutoPin()
    const String& build() {
      if (config.length() == 2) {
        // just started
        config.concat('*');
      }
      config.concat(')');
      return config;
    }  
  private:  
    void addConfig(const String& conf) {
      if (started[depth]) {
        config.concat('+');
      } else {
        started[depth] = true;
      }
      config.concat(conf);
    }
  private:
    int depth;
    bool *started;
    String config;
};




/// @brief
/// Helper class for managing layer layout plus update of the layers.
/// @since v0.9.7-r2
class DDLayoutHelper {
  public: 
    DDLayoutHelper(DumbDisplay& dumbdisplay): dumbdisplay(dumbdisplay), versionTracker(-1) {}
  public:
    /// check whether layers need be updated, say
    /// - just initialzed
    /// - DD reconnected
    bool checkNeedToUpdateLayers(/*DumbDisplay& dumbdisplay*/) {
      DDYield();
      return versionTracker.checkChanged(dumbdisplay);
    }
    /// essentially DumbDisplay::recordLayerSetupCommands()
    /// *MUST* call finishInitializeLayout() layout when done
    inline void startInitializeLayout(/*DumbDisplay& dumbdisplay*/) {
      dumbdisplay.recordLayerSetupCommands();
    }
    /// after calling startInitializeLayout(), call this to finish the layout of layers
    /// @param layerSetupPersistId is use for calling DumbDisplay.playbackLayerSetupCommands()
    inline void finishInitializeLayout(/*DumbDisplay& dumbdisplay, */String layerSetupPersistId) {
      dumbdisplay.playbackLayerSetupCommands(layerSetupPersistId);
    }
    /// basically DumbDisplay::configAutoPin()
    inline void configAutoPin(const String& layoutSpec) {
      dumbdisplay.configAutoPin(layoutSpec);
    }
    /// basically DumbDisplay::addRemainingAutoPinConfig()
    inline void addRemainingAutoPinConfig(const String& remainingLayoutSpec) {
      dumbdisplay.addRemainingAutoPinConfig(remainingLayoutSpec);
    }
    /// basically DumbDisplay::configPinFrame()
    inline void configPinFrame(int xUnitCount = 100, int yUnitCount = 100) {
      dumbdisplay.configPinFrame(xUnitCount, yUnitCount);
    }
    /// basically DumbDisplay::pinLayer()
    inline void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "") {
      dumbdisplay.pinLayer(pLayer, uLeft, uTop, uWidth, uHeight, align);
    }
    /// basically DumbDisplay::pinAutoPinLayers()
    inline void pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "") {
      dumbdisplay.pinAutoPinLayers(layoutSpec, uLeft, uTop, uWidth, uHeight, align);
    }
#ifndef DD_NO_IDLE_CALLBACK
    /// basically DumbDisplay::setIdleCallback()
    inline void setIdleCallback(DDIdleCallback idleCallback) {
      dumbdisplay.setIdleCallback(idleCallback);
    }
#endif
    // // deprecated
    // inline void setIdleCalback(DDIdleCallback idleCallback) {
    //   dumbdisplay.setIdleCalback(idleCallback);
    // }
  private:
    DumbDisplay& dumbdisplay;
    DDConnectVersionTracker versionTracker;
};



