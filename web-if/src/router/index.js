import Vue from "vue";
import Router from "vue-router";
import LoginForm from "../forms/LoginForm.vue"
import PhotoForm from "../forms/PhotoForm.vue"
import SettingsForm from "../forms/SettingsForm.vue"
import FrequencyForm from "../forms/FrequencyForm.vue"
import store from "../store";

Vue.use(Router);

const ifNotAuthenticated = (to, from, next) => {
  if (!store.getters.isAuthenticated) {
    next();
  } else {
    next("/login");
  }
};

const ifAuthenticated = (to, from, next) => {
  if (store.getters.isAuthenticated) {
    next();
  } else {
    next("/login");
  }
};


export default new Router({
  mode: "history",
  routes: [
    {
        path: '/',
        redirect: {
            name: "photos"
        }
    },
    {
      path: "/photos",
      name: "photos",
      component: PhotoForm,
      beforeEnter: ifAuthenticated
    },
    {
      path: "/settings",
      name: "settings",
      component: SettingsForm,
      beforeEnter: ifAuthenticated
    },
    {
      path: "/frequency",
      name: "frequency",
      component: FrequencyForm,
      beforeEnter: ifAuthenticated
    },
    {
      path: "/login",
      name: "Login",
      component: LoginForm,
      beforeEnter: ifNotAuthenticated
    },
  ]
});
