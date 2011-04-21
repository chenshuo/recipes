package billing;

import static billing.DataFields.*;

public class NormalUserNewJoinRule extends Rule {

    public static final long kFreePhoneCallMinutes = 60;
    public static final long kFreeShortMessages = 200;
    public static final long kFreeInternetKiloBytes = 50*1000L;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kNormal;
        boolean isNew = input.getBoolean(UserField.kIsNewUser);
        assert isNew;
        List<Package> packages = input.getPackages();
        assert packages.isEmpty();

        Rule phoneCall = factory.create(type.getRuleName()+"_phone_call", kFreePhoneCallMinutes);
        Rule shortMessage = factory.create(type.getRuleName()+"_short_message", kFreeShortMessages);
        Rule internet = factory.create(type.getRuleName()+"_internet", kFreeInternetKiloBytes);
        long total = phoneCall.getMoneyInPips(input) +
                     shortMessage.getMoneyInPips(input) +
                     internet.getMoneyInPips(input);
        return total;
    }
}
