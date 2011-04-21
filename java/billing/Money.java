package billing;

public class Money {
    public static final long kPipsPerCent = 100;
    public static final long kCentsPerYuan = 100;
    public static final long kPipsPerYuan = kPipsPerCent * kCentsPerYuan;

    public final long pips;

    public Money(long pips) {
        this.pips = pips;
    }

    public static Money get(int yuan, int cents) {
        return new Money(yuan * kPipsPerYuan + cents * kPipsPerCent);
    }

    public long getCentsRoundDown() {
        return pips / kPipsPerCent;
    }

    public long getCentsRoundUp() {
        return (pips + kPipsPerCent - 1) / kPipsPerCent;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof Money) {
            Money that = (Money) obj;
            return pips == that.pips;
        } else {
            return false;
        }
    }

    @Override
    public int hashCode() {
        return (int) pips;
    }

    @Override
    public String toString() {
        return Long.toString(pips) + " pips";
    }
}
