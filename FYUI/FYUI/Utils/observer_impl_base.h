#pragma once
#include <map>

template <typename ReturnT, typename ParamT>
class ReceiverImplBase;

template <typename ReturnT, typename ParamT>
class ObserverImplBase
{
public:
	/**
	 * @brief 添加Receiver
	 * @details 用于添加Receiver。具体行为由当前对象状态以及传入参数共同决定。
	 * @param receiver [in] receiver参数
	 */
	virtual void AddReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
	/**
	 * @brief 移除Receiver
	 * @details 用于移除Receiver。具体行为由当前对象状态以及传入参数共同决定。
	 * @param receiver [in] receiver参数
	 */
	virtual void RemoveReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver) = 0;
	/**
	 * @brief 执行 Broadcast 操作
	 * @details 用于执行 Broadcast 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @return ReturnT 返回 执行 Broadcast 操作 的结果
	 */
	virtual ReturnT Broadcast(ParamT param) = 0;
	/**
	 * @brief 执行 Notify 操作
	 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @return ReturnT 返回 执行 Notify 操作 的结果
	 */
	virtual ReturnT Notify(ParamT param) = 0;
};

template <typename ReturnT, typename ParamT>
class ReceiverImplBase
{
public:
	/**
	 * @brief 添加Observer
	 * @details 用于添加Observer。具体行为由当前对象状态以及传入参数共同决定。
	 * @param observer [in] observer参数
	 */
	virtual void AddObserver(ObserverImplBase<ReturnT, ParamT>* observer) = 0;
	/**
	 * @brief 移除Observer
	 * @details 用于移除Observer。具体行为由当前对象状态以及传入参数共同决定。
	 */
	virtual void RemoveObserver() = 0;
	/**
	 * @brief 执行 Receive 操作
	 * @details 用于执行 Receive 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @return ReturnT 返回 执行 Receive 操作 的结果
	 */
	virtual ReturnT Receive(ParamT param) = 0;
	/**
	 * @brief 执行 Respond 操作
	 * @details 用于执行 Respond 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @param observer [in] observer参数
	 * @return ReturnT 返回 执行 Respond 操作 的结果
	 */
	virtual ReturnT Respond(ParamT param, ObserverImplBase<ReturnT, ParamT>* observer) = 0;
};

template <typename ReturnT, typename ParamT>
class ReceiverImpl;

template <typename ReturnT, typename ParamT>
class ObserverImpl : public ObserverImplBase<ReturnT, ParamT>
{
public:
	/**
	 * @brief 构造 ObserverImpl 对象
	 * @details 用于构造 ObserverImpl 对象。具体行为由当前对象状态以及传入参数共同决定。
	 */
	ObserverImpl()
		: count_(0)
	{}

	virtual ~ObserverImpl()	{}

	virtual void AddReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver)
	{
		if (receiver == NULL)
			return;

		receivers_[count_] = receiver;
		receiver->AddObserver(this);
		count_++;
	}

	/**
	 * @brief 移除Receiver
	 * @details 用于移除Receiver。具体行为由当前对象状态以及传入参数共同决定。
	 * @param receiver [in] receiver参数
	 */
	virtual void RemoveReceiver(ReceiverImplBase<ReturnT, ParamT>* receiver)
	{
		if (receiver == NULL)
			return;

		typename ReceiversMap::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			if (it->second == receiver)
			{
				receivers_.erase(it);
				break;
			}
		}
	}

	/**
	 * @brief 执行 Broadcast 操作
	 * @details 用于执行 Broadcast 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @return ReturnT 返回 执行 Broadcast 操作 的结果
	 */
	virtual ReturnT Broadcast(ParamT param)
	{
		typename ReceiversMap::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			it->second->Receive(param);
		}

		return ReturnT();
	}

	/**
	 * @brief 执行 Notify 操作
	 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @return ReturnT 返回 执行 Notify 操作 的结果
	 */
	virtual ReturnT Notify(ParamT param)
	{
		typename ReceiversMap::iterator it = receivers_.begin();
		for (; it != receivers_.end(); ++it)
		{
			it->second->Respond(param, this);
		}

		return ReturnT();
	}

protected:
	typedef std::map<int, ReceiverImplBase<ReturnT, ParamT>*> ReceiversMap;
	ReceiversMap receivers_;
	int count_;
};


template <typename ReturnT, typename ParamT>
class ReceiverImpl : public ReceiverImplBase<ReturnT, ParamT>
{
public:
	/**
	 * @brief 构造 ReceiverImpl 对象
	 * @details 用于构造 ReceiverImpl 对象。具体行为由当前对象状态以及传入参数共同决定。
	 */
	ReceiverImpl()
		: count_(0)
	{}

	virtual ~ReceiverImpl()	{}

	virtual void AddObserver(ObserverImplBase<ReturnT, ParamT>* observer)
	{
		observers_[count_] = observer;
		count_++;
	}

	/**
	 * @brief 移除Observer
	 * @details 用于移除Observer。具体行为由当前对象状态以及传入参数共同决定。
	 */
	virtual void RemoveObserver()
	{
		typename ObserversMap::iterator it = observers_.begin();
		for (; it != observers_.end(); ++it)
		{
			it->second->RemoveReceiver(this);
		}
	}

	/**
	 * @brief 执行 Receive 操作
	 * @details 用于执行 Receive 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @return ReturnT 返回 执行 Receive 操作 的结果
	 */
	virtual ReturnT Receive(ParamT param)
	{
		return ReturnT();
	}

	/**
	 * @brief 执行 Respond 操作
	 * @details 用于执行 Respond 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param param [in] 回调参数
	 * @param observer [in] observer参数
	 * @return ReturnT 返回 执行 Respond 操作 的结果
	 */
	virtual ReturnT Respond(ParamT param, ObserverImplBase<ReturnT, ParamT>* observer)
	{
		return ReturnT();
	}

protected:
	typedef std::map<int, ObserverImplBase<ReturnT, ParamT>*> ObserversMap;
	ObserversMap observers_;
	int count_;
};
