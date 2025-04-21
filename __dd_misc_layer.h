
/// Utility class 
template<int MAX_LAYER_COUNT>
class DDFadingLayers {
  public:
    DDFadingLayers() {
      this->layerCount = 0;
      this->nextUseLayerIdx = 0;
    }
    void initAddLayer(DDLayer* layer) {
      if (layerCount < MAX_LAYER_COUNT) {
        layers[layerCount++] = layer;
      }
    }
    void clear() {
      for (int i = 0; i < layerCount; i++) {
        layers[i]->clear();
      }
      nextUseLayerIdx = 0;
    }
  public:  
    DDLayer* useLayer() {
      int layerIdx = nextUseLayerIdx;
      nextUseLayerIdx = (nextUseLayerIdx + 1) % layerCount;
      for (int i = 0; i < this->layerCount; i++) {
        int lidx = (layerIdx + i) % layerCount;
        DDLayer* layer = layers[lidx];
        if (i == 0) {
          layer->opacity(100);
          layer->clear();
        } else {
          int opacity = i * (100.0 / layerCount);
          layer->opacity(opacity);
        }
      }
      return layers[layerIdx];
    }  
  private:
    DDLayer* layers[MAX_LAYER_COUNT];
    int layerCount;
    int nextUseLayerIdx;
};


/// helper for using of SelectionListLayerWrapper; suggested to use SelectionListLayerWrapper whenever it makes sense
class SelectionListLayerHelper {
  public:
    SelectionListLayerHelper(SelectionListDDLayer* selectionListLayer): selectionListLayer(selectionListLayer) {
      this->selectionCount = 0;
      this->selectionOffset = 0;
      this->viewSelectionCount = -1;
      this->listStateChangedCallback = NULL;
    }
  public:
    inline SelectionListDDLayer* getLayer() {
      return selectionListLayer;
    }
    inline int getOffset() {
      return selectionOffset;
    }
    inline int getSelectionCount() {
      return selectionCount;
    }
  public:
    /// set the "list state changed" callback; here "state" refers to "list add/remove text or change of offset"
    void setListStateChangedCallback(void (*listStateChangedCallback)(), int viewSelectionCount = -1) {
      this->listStateChangedCallback = listStateChangedCallback;
      this->viewSelectionCount = viewSelectionCount;
    }
  public:
    /// @param selectionIdx -1 means append as the last selection
    /// @return -1 if selectionIdx is out of range
    int add(int selectionIdx = -1) {
      int addSelectionIdx;
      if (selectionIdx == -1) {
        addSelectionIdx = selectionCount;
      } else {
        addSelectionIdx = selectionIdx;
      } 
      if (addSelectionIdx < 0 || addSelectionIdx > selectionCount) {
        return -1;
      }
      selectionListLayer->add(addSelectionIdx);
      selectionCount += 1;
      onListStateChanged();
      return addSelectionIdx;
    }  
    /// @param selectionIdx -1 means the last selection
    /// @return -1 if selectionIdx is out of range
    int remove(int selectionIdx = -1) {
      int removeSelectionIdx;
      if (selectionIdx == -1) {
        removeSelectionIdx = selectionCount;
      } else {
        removeSelectionIdx = selectionIdx;
      } 
      if (removeSelectionIdx < 0 || removeSelectionIdx >= selectionCount) {
        return -1;
      }
      selectionListLayer->remove(removeSelectionIdx);
      selectionCount -= 1;
      if (selectionOffset > selectionCount) {
        selectionOffset = selectionCount;
      }
      onListStateChanged();
      return removeSelectionIdx;
    }
    void setOffset(int offset) {
      selectionListLayer->offset(offset);
      selectionOffset = offset;
      onListStateChanged();
    }
    inline void decrementOffset() {
      setOffset(selectionOffset - 1);
    }
    inline void incrementOffset() {
      setOffset(selectionOffset + 1);
    }
    void scrollToView(int selectionIdx) {
      if (selectionIdx < selectionOffset) {
        setOffset(selectionIdx);
      } else {
        if (this->viewSelectionCount != -1) {
          if (selectionIdx >= (selectionOffset + this->viewSelectionCount)) {
            setOffset(selectionIdx - this->viewSelectionCount + 1);
          }
        }
      }  
    }
  protected:  
    void onListStateChanged() {
      if (this->listStateChangedCallback != NULL) {
        this->listStateChangedCallback();
      }
      // if (scrollUpLayer != NULL) {
      //   bool canScrollUp = selectionOffset > 0;
      //   scrollUpLayer->disabled(!canScrollUp);
      // }
      // if (scrollDownLayer != NULL) {
      //   bool canScrollDown = selectionOffset < (selectionCount - visibleSelectionCount);
      //   scrollDownLayer->disabled(!canScrollDown);
      // }
    }  
  private:  
    SelectionListDDLayer* selectionListLayer;
    int selectionCount;
    int selectionOffset;
    int viewSelectionCount;
    void (*listStateChangedCallback)();
    //DDLayer* scrollUpLayer;
    //DDLayer* scrollDownLayer;
};


/// wrapper for using of SelectionListLayerWrapper that help you to track the text added
class SelectionListLayerWrapper {
  public:
    SelectionListLayerWrapper(short bufferSizeInc = 2) {
      this->bufferSizeInc = bufferSizeInc;
      this->helper = NULL;
      this->textBuffer = NULL;
      this->textBufferSize = 0;
      this->trackedTextCount = 0;
    }
    /// initialize a new SelectionListDDLayer (note that any old ones will not be deleted)
    SelectionListDDLayer* initializeLayer(DumbDisplay& dumbdisplay,
                                          int colCount, int rowCount,
                                          int horiSelectionCount, int vertSelectionCount,
                                          int charHeight = 0, const String& fontName = "",
                                          bool canDrawDots = true, float selectionBorderSizeCharHeightFactor = 0.3) {
      if (this->helper != NULL) {
        delete this->helper;
        this->helper = NULL;
      }
      if (this->textBuffer != NULL) {
        delete[] this->textBuffer;
        this->textBuffer = NULL;
        this->textBufferSize = 0;
        this->trackedTextCount = 0;
      }
      SelectionListDDLayer* selectionListLayer = dumbdisplay.createSelectionListLayer(colCount, rowCount, horiSelectionCount, vertSelectionCount, charHeight, fontName, canDrawDots, selectionBorderSizeCharHeightFactor);
      this->helper = new SelectionListLayerHelper(selectionListLayer);
      this->viewHoriSelectionCount = horiSelectionCount;
      this->viewSelectionCount = horiSelectionCount * vertSelectionCount;
      // setScrollLayers(NULL, NULL);
      return selectionListLayer;
    }
  public:   
    inline SelectionListDDLayer* getLayer() {
      return helper->getLayer();
    }
    inline int getOffset() {
      return helper->getOffset();
    }
    inline int getSelectionCount() {
      return helper->getSelectionCount();
    }
    int getSelectionIndexFromView(int horiSelectionIdx, int vertSelectionIdx) {
      return this->getOffset() + (vertSelectionIdx * this->viewHoriSelectionCount) + horiSelectionIdx;
    }
    const String& getSelectionText(int selectionIdx) {
      return this->textBuffer[selectionIdx];
    }
    int findSelection(const String& text) {
      for (int i = 0; i < trackedTextCount; i++) {
        if (textBuffer[i] == text) {
          return i;
        }
      }
      return -1;
    }
  public:   
    /// set the "list state changed" callback; here "state" refers to "list add/remove text or change of offset"
    void setListStateChangedCallback(void (*listStateChangedCallback)()) {
      helper->setListStateChangedCallback(listStateChangedCallback, this->viewSelectionCount);
    }
    /// @param selectionIdx -1 means append as the last selection
    /// @return -1 if selectionIdx is out of range
    int addSelection(int selectionIdx, const String& text, const String& align = "L") {
      int idx = helper->add(selectionIdx);
      if (idx != -1) {
        helper->getLayer()->text(idx, text, 0, align);
        if ((trackedTextCount + 1) > textBufferSize) {
          int newSize = textBufferSize + bufferSizeInc;
          String* newTextBuffer = new String[newSize];
          for (int i = 0; i < textBufferSize; i++) {
            newTextBuffer[i] = textBuffer[i];
          }
          if (textBuffer != NULL) {
            delete[] textBuffer;
          }
          textBuffer = newTextBuffer;
          textBufferSize = newSize;
        }
        textBuffer[trackedTextCount++] = text;
      }
      return idx;
    }
    /// @param selectionIdx -1 means the last selection
    /// @return -1 if selectionIdx is out of range
    int removeSelection(int selectionIdx) {
      selectionIdx = helper->remove(selectionIdx);
      if (selectionIdx != -1) {
        trackedTextCount -= 1;
        for (int i = selectionIdx; i < trackedTextCount; i++) {
          textBuffer[i] = textBuffer[i + 1];
        }
      }
      return selectionIdx;
    }
    void removeAllSelections() {
      int nameCount = getSelectionCount();
      for (int i = 0; i < nameCount; i++) {
        removeSelection(0);
      }
    }
    void setOffset(int offset) {
      helper->setOffset(offset);
    }
    inline void incrementOffset() {
      helper->incrementOffset();
    }
    inline void decrementOffset() {
      helper->decrementOffset();
    }
    void scrollToView(int selectionIdx) {
      helper->scrollToView(selectionIdx);
    }
    void select(int selectionIdx, bool deselectTheOthers = true) {
      helper->getLayer()->select(selectionIdx, deselectTheOthers);
    }
    void deselect(int selectionIdx, bool selectTheOthers = false) {
      helper->getLayer()->deselect(selectionIdx, selectTheOthers);
    }
    void selected(int selectionIdx, bool selected, bool reverseTheOthers = false) {
      helper->getLayer()->selected(selectionIdx, selected, reverseTheOthers);
    }
    void selectAll() {
      helper->getLayer()->selectAll();
    }
    void deselectAll() {
      helper->getLayer()->deselectAll();
    }
  private: 
    SelectionListLayerHelper* helper; 
    String* textBuffer;
    short bufferSizeInc;
    int textBufferSize;
    int trackedTextCount;
    int viewHoriSelectionCount;
    int viewSelectionCount;
};

