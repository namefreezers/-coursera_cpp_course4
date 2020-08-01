
namespace RAII {

    template<typename BookingProvider>
    class Booking {

    public:
        Booking(BookingProvider *provider_ptr, int id) : provider_ptr_(provider_ptr), id_(id), is_captured_(true) {}

        Booking(const Booking &other) = delete;

        Booking(Booking &&other) : provider_ptr_(other.provider_ptr_), id_(other.id_), is_captured_(other.is_captured_) {
            other.is_captured_ = false;
        }

        Booking &operator=(const Booking &other) = delete;

        Booking &operator=(Booking &&other) noexcept {
            if (this != &other) {
                this->Release();

                this->id_ = other.id_;
                this->provider_ptr_ = other.provider_ptr_;
                this->is_captured_ = other.is_captured_;

                other.is_captured_ = false;
            }
            return *this;
        }

        ~Booking() {
            Release();
        }

    private:
        void Release() {
            if (is_captured_) {
                provider_ptr_->CancelOrComplete(*this);
                is_captured_ = false;
            }
        }

        BookingProvider *provider_ptr_;
        int id_;
        bool is_captured_;
    };

}