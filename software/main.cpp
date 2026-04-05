


	#include <JuceHeader.h>


	class Visualizer : public juce::Component, private juce::Timer
	{
	public:
	
		 juce::String name = "";
		 
		 float* pbuf=0;
		 
		 juce::Path linePath;
		 
		 float fps=0;
		 int frc=0; // frames counter
		 
		 Visualizer() {
			 linePath.preallocateSpace(256);
			 startTimerHz(1);
			}
			
		 void paint(juce::Graphics& g) override {
			 
			  if(!pbuf) return;
			  
			  auto w = (float)getWidth();
			  auto h = (float)getHeight();

			  g.fillAll( juce::Colours::black );
			  
			  g.setColour( juce::Colours::white );
			  linePath.startNewSubPath( 0, h/2.0 );
			  linePath.lineTo( w, h/2.0 );
			  g.strokePath( linePath, juce::PathStrokeType(1.0f) );
			  linePath.clear();
			  
			  g.setColour(juce::Colours::cyan);
			  for( int i = 0; i < 256; ++i ){
				  
					float x = (float)i * (w / 255.0f);
					
					float y = -pbuf[i]; // invert
					y *= h/2.0; // scale
					
					if( y> h/2.0 ) y=h/2.0;
					if( y< -h/2.0 ) y=-h/2.0;

					if( !i )
						linePath.startNewSubPath( x, h/2.0 +y );
					else
						linePath.lineTo( x, h/2.0 +y );
			  }

			  g.strokePath(linePath, juce::PathStrokeType(2.0f));
			  linePath.clear();
			  
				// 3. Изписваш текст
				g.setColour(juce::Colours::white); // Цвят на текста
				g.setFont(18.0f);                 // Размер на шрифта
				g.drawText( name + " fps " + juce::String( fps ), 10, 10, 200, 30, juce::Justification::left );

			  repaint();
			  frc ++;
		 }


		void timerCallback() override { // 1sec
			fps = frc;
			frc = 0;
		}
	};



	inline Colour getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback = Colour (0xff4d4d4d)) noexcept {
		 if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
			  return v4->getCurrentColourScheme().getUIColour (uiColour);
		 return fallback; }


	class AudioSettingsDemo final : public Component, private juce::Timer  {
	  
	private:

		 TextEditor textbox;
		 Visualizer vis1;
		 Visualizer vis2;

		 juce::DatagramSocket socket { false }; 
		 int64_t totalBytes = 0;
		 int64_t ticks_per_second;
		 int64_t T0, t_last_print, bytes_last_print;  // in "high resolution ticks"
		 
		 uint8_t rcv_buf [1024];  //  1024 bytes; 512 int16; 256 samples per channel
		 
		 float left_buf [256];
		 float right_buf [256];
		 
		 float lavg, ravg;
		 float lavg_, ravg_; // after -1..+1 scaling
		 
	public:
	
		 AudioSettingsDemo()
				{

			  setOpaque( true );

			  addAndMakeVisible( textbox );
			  addAndMakeVisible( vis1 );
			  addAndMakeVisible( vis2 );
			  vis1.name = "Left ch";
			  vis1.pbuf = left_buf;
			  vis2.name = "Right ch";
			  vis2.pbuf = right_buf;
				
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
					logMessage (" bound to port 4444");
			  } else {
					logMessage( "Error: Could not bind to port 4444" ); }

			  setSize (500, 600); }

		 void paint (Graphics& g) override {
			  g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
			  }

		 void resized() override {
			  auto r =  getLocalBounds().reduced (10);
			  auto r1 = r; r1.removeFromBottom( r1.getHeight()/2 );
			  auto r2 = r; r2.removeFromTop( r2.getHeight()/2 );
			  textbox.setBounds (r1);

			  auto rr1 = r2; rr1.removeFromBottom( rr1.getHeight()/2 );
			  auto rr2 = r2; rr2.removeFromTop( rr2.getHeight()/2 );

			  vis1.setBounds(rr1.reduced(5));
			  vis2.setBounds(rr2.reduced(5));
			  
			  }

		 void logMessage (const String& m) {
			  textbox.moveCaretToEnd();
			  textbox.insertTextAtCaret (m + newLine); }

		 ~AudioSettingsDemo() override {
			  }

	private:

		 void timerCallback() override {

			int64_t b1 = totalBytes;
				
			int bytesRead = 0;
			while( (bytesRead = socket.read( rcv_buf, 1024, false )) > 0) {
			  totalBytes += bytesRead;
			  }

			 if( totalBytes -b1 > 0 ){
				 
				{// unintereleave
				uint16_t *p = (uint16_t *) rcv_buf;
				for( int i=0; i<512; i += 2 ){
					left_buf[i/2] = (float)(p[i]);
					right_buf[i/2] = (float)(p[i+1]);
					}
				}

				// avg
				lavg=ravg=0.0;
				for( int i=0; i<256; i++ ){
					lavg += left_buf[i] ;
					ravg += right_buf[i] ; }
				lavg /= 256.0; ravg /= 256.0;

				{// scale
				for( int i=0; i<256; i ++ ){
					left_buf[i] = (left_buf[i]-2048.0) /2048.0;
					right_buf[i] = (right_buf[i]-2048.0) /2048.0;
					}
				}

				// avg_
				lavg_=ravg_=0.0;
				for( int i=0; i<256; i++ ){
					lavg_ += left_buf[i] ;
					ravg_ += right_buf[i] ; }
				lavg_ /= 256.0; ravg_ /= 256.0;

			  
			 }			  

			  // print on 1sec
			  int64_t now = juce::Time::getHighResolutionTicks();
			  float seconds_since_last_print = ((float)(now -t_last_print)) / ((float) ticks_per_second );
			  if( seconds_since_last_print > 1.0 && bytes_last_print != totalBytes ){
				   logMessage( "Bytes received: " + juce::String( totalBytes ));
					bytes_last_print = totalBytes;
					t_last_print = now;
					logMessage( " L " + juce::String( (int)lavg ) + " (" + juce::String( lavg_, 2 ) + ")" );
					logMessage( " R " + juce::String( (int)ravg ) + " (" + juce::String( ravg_, 2 ) + ")" );
					logMessage( "" );
					}

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
