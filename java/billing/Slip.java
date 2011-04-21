package billing;

import org.joda.time.LocalDateTime;

import billing.DataFields.SlipType;

public class Slip {
    public final SlipType type;
    public final LocalDateTime time;
    // DateTimeZone zone;
    // Map<String, Data> data
    long data;

    public Slip(SlipType type, LocalDateTime time, long data) {
        this.type = type;
        this.time = time;
        this.data = data;
    }

}
