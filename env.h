
class Environment
{
public:
	class  AsyncHooks
	{
	public:
		inline unsigned int* fileds();
		inline int fields_count() const;
		inline bool callbacks_enabled();
		inline void set_enable_callbacks(unsigned int flag);

	private:
		friend class Environment;
		inline AsyncHooks();
		enum Fields {
			kEnableCallbacks,
			kFieldCount
		};
	};


private:

};
