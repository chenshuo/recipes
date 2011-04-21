package billing;

import static billing.DataFields.*;

public class VipUserNewJoinRule extends Rule {
    public static final long kFreePhoneCallMinutes = 200;
    public static final long kFreeShortMessages = 200;
    public static final long kFreeInternetKiloBytes = 100*1000L;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kVip;
        boolean isNew = input.getBoolean(UserField.kIsNewUser);
        assert isNew;
        List<Package> packages = input.getPackages();
        assert packages.isEmpty();

        Rule rule = factory.create("vip_user_nopackage",
            [ kFreePhoneCallMinutes, kFreeShortMessages, kFreeInternetKiloBytes ]);
        return rule.getMoneyInPips(input);
    }
}
