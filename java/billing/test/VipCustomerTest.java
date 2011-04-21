package billing.test;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.joda.time.LocalDateTime;
import org.junit.Test;

import billing.BillCalculator;
import billing.Money;
import billing.Slip;
import billing.UserMonthUsage;
import billing.DataFields.PackageType;
import billing.DataFields.SlipType;
import billing.DataFields.UserField;
import billing.DataFields.UserType;

public class VipCustomerTest {

    private static final BillCalculator calculator = new BillCalculator("./groovy/billing/");

    @Test
    public void testVipUserNoPackageNoActivity() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(60, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNoPackageNoActivity28days() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 28);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(56, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNoPackageOneCall() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 1));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(60, 40), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNoPackageOneMessage() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 1));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(60, 10), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNoPackage50kInternet() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 50));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(60, 15), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNoPackageAllThree() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 10));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 15));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(65, 80), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNoPackageAllThree31Days() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 31);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 10));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 15));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(67, 80), calculator.calculate(usage));
    }

    public List<PackageType> getPackages(PackageType... packages) {
        return Arrays.asList(packages);
    }

    @Test
    public void testVipUserPackage1NoActivity() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage1));
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(100, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage1WithInLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage1));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 750));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(100, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage1OneMoreCall() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage1));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 751));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(100, 30), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage1OneMoreMessage() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage1));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 750));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 201));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(100, 10), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage1And50kMore() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage1));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 750));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000 + 50));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(100, 5), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage1AllExceed() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage1));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 751));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 201));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000 + 50));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(100, 45), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage2NoActivity() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage2));
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(200, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage2WithInLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage2));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 2000));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 500));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 300 * 1000));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(200, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserPackage2More() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 30);
        data.put(UserField.kPackages, getPackages(PackageType.kVipUserPackage2));
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 2001));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 501));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 300 * 1000 + 100));
        data.put(UserField.kSlips, slips);
        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(200, 35), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNewJoinNoActivities() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 12);

        data.put(UserField.kIsNewUser, true);

        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(24, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNewJoinWithInLimit() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 15);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 200));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kIsNewUser, true);

        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(30, 0), calculator.calculate(usage));
    }

    @Test
    public void testVipUserNewJoinMore() {
        Map<UserField, Object> data = new HashMap<UserField, Object>();
        data.put(UserField.kUserType, UserType.kVip);
        data.put(UserField.kDaysServed, 15);
        List<Slip> slips = new ArrayList<Slip>();
        slips.add(new Slip(SlipType.kPhoneCall, new LocalDateTime(), 201));
        slips.add(new Slip(SlipType.kShortMessage, new LocalDateTime(), 201));
        slips.add(new Slip(SlipType.kInternet, new LocalDateTime(), 100 * 1000 + 100));
        data.put(UserField.kSlips, slips);

        data.put(UserField.kIsNewUser, true);

        UserMonthUsage usage = new UserMonthUsage(1, data);
        assertEquals(Money.get(30, 80), calculator.calculate(usage));
    }
}
