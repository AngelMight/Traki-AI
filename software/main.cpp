


	#include <JuceHeader.h>


	inline Colour getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback = Colour (0xff4d4d4d)) noexcept {
		 if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
			  return v4->getCurrentColourScheme().getUIColour (uiColour);
		 return fallback; }


	class AudioSettingsDemo final : public Component, private juce::Timer  {
	  
	private:
	
		 TextEditor textbox;
		 
		 juce::DatagramSocket socket { false }; 
		 int64_t totalBytes = 0;
		 int64_t ticks_per_second;
		 int64_t T0, t_last_print, bytes_last_print;  // in "high resolution ticks"
		 
		 
	public:
	
		 AudioSettingsDemo() {
			 
			  setOpaque( true );

			  addAndMakeVisible( textbox );
			  
			  textbox.setMultiLine (true);
			  textbox.setReturnKeyStartsNewLine (true);
			  textbox.setReadOnly (true);
			  textbox.setScrollbarsShown (true);
			  textbox.setCaretVisible (false);
			  textbox.setPopupMenuEnabled (true);

				ticks_per_second = juce::Time::getHighResolutionTicksPerSecond();
				T0 = juce::Time::getHighResolutionTicks();
				t_last_print = T0;
				bytes_last_print = 0;

			  logMessage ("hello:\n");

			  if( socket.bindToPort( 4444 ) ) {
					startTimer( 1 ); // Проверка за нови пакети на всеки 1ms
					logMessage (" bind to port 4444");
			  } else {
					logMessage( "Error: Could not bind to port 4444" ); }

			  setSize (500, 600); }

		 void paint (Graphics& g) override {
			  g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground)); }

		 void resized() override {
			  auto r =  getLocalBounds().reduced (4);
			  textbox.setBounds (r); }

		 void logMessage (const String& m) {
			  textbox.moveCaretToEnd();
			  textbox.insertTextAtCaret (m + newLine); }

		 ~AudioSettingsDemo() override {
			  }

	private:

		 void timerCallback() override {

			  char buffer[ 2048 ];
			  int bytesRead = 0;

			  while( (bytesRead = socket.read( buffer, sizeof(buffer), false )) > 0) {
					totalBytes += bytesRead;
					 }
			  
			  int64_t now = juce::Time::getHighResolutionTicks();
			  float seconds_since_last_print = ((float)(now -t_last_print)) / ((float) ticks_per_second );
			  if( seconds_since_last_print > 1.0 && bytes_last_print != totalBytes ){
				   logMessage( "Bytes received: " + juce::String( totalBytes ));
					bytes_last_print = totalBytes;
					t_last_print = now; }

		 }
		 
		 void lookAndFeelChanged() override {
			  textbox.applyFontToAllText( textbox.getFont() ); }

			  
		 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsDemo)
	};






	class Application    : public juce::JUCEApplication
	{
	public:
		 //==============================================================================
		 Application() = default;

		 const juce::String getApplicationName() override       { return "AudioSettingsDemo"; }
		 const juce::String getApplicationVersion() override    { return "1.0.0"; }

		 void initialise (const juce::String&) override
		 {
			  mainWindow.reset (new MainWindow ("AudioSettingsDemo", std::make_unique<AudioSettingsDemo>(), *this));
		 }

		 void shutdown() override                         { mainWindow = nullptr; }

	private:
		 class MainWindow    : public juce::DocumentWindow
		 {
		 public:
			  MainWindow (const juce::String& name, std::unique_ptr<juce::Component> c, JUCEApplication& a)
					: DocumentWindow (name, juce::Desktop::getInstance().getDefaultLookAndFeel()
																						 .findColour (ResizableWindow::backgroundColourId),
											juce::DocumentWindow::allButtons),
					  app (a)
			  {
					setUsingNativeTitleBar (true);

				  #if JUCE_ANDROID || JUCE_IOS
					setContentOwned (new SafeAreaComponent { std::move (c) }, true);
					setFullScreen (true);
				  #else
					setContentOwned (c.release(), true);
					setResizable (true, false);
					setResizeLimits (300, 250, 10000, 10000);
					centreWithSize (getWidth(), getHeight());
				  #endif

					setVisible (true);
			  }

			  void closeButtonPressed() override
			  {
					app.systemRequestedQuit();
			  }

			 #if JUCE_ANDROID || JUCE_IOS
			  class SafeAreaComponent : public juce::Component
			  {
			  public:
					explicit SafeAreaComponent (std::unique_ptr<Component> c)
						 : content (std::move (c))
					{
						 addAndMakeVisible (*content);
					}

					void resized() override
					{
						 if (const auto* d = Desktop::getInstance().getDisplays().getDisplayForRect (getLocalBounds()))
							  content->setBounds (d->safeAreaInsets.subtractedFrom (getLocalBounds()));
					}

			  private:
					std::unique_ptr<Component> content;
			  };

			  void parentSizeChanged() override
			  {
					if (auto* c = getContentComponent())
						 c->resized();
			  }
			 #endif

		 private:
			  JUCEApplication& app;

			  //==============================================================================
			  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
		 };

		 std::unique_ptr<MainWindow> mainWindow;
	};

	//==============================================================================
	START_JUCE_APPLICATION (Application)
